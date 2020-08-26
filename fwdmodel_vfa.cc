/*  fwdmodel_VFA.cc - Implements T1 estimation using the Variable Flip Angle approach

    Jesper Kallehauge, IBME

    Modified by: Alex Smith, FMRIB, 20171206

    Copyright (C) 2007-2017 University of Oxford  */

/*  CCOPYRIGHT */

#include "fwdmodel_vfa.h"

#include "fabber_core/tools.h"
#include "miscmaths/miscprob.h"

#include "armawrap/newmat.h"

#include <string>
#include <vector>

using namespace NEWMAT;
using namespace std;

FactoryRegistration<FwdModelFactory, VFAFwdModel> VFAFwdModel::registration("vfa");

static OptionSpec OPTIONS[] = {
    { "tr", OPT_FLOAT, "TR in seconds for VFA images", OPT_REQ, "" },
    { "fas-file", OPT_MATRIX, "File containing a list of flip angles", OPT_NONREQ, "" },
    { "fa<n>", OPT_FLOAT, "Alternative to fas-file, specify a sequence of flip angles --fa1=12 --fa2=15 etc", OPT_NONREQ, "" },
    { "radians", OPT_BOOL, "If specified, flip angles are given in radians", OPT_NONREQ, "" },
    { "" }
};

void VFAFwdModel::GetOptions(vector<OptionSpec> &opts) const
{
    for (int i = 0; OPTIONS[i].name != ""; i++)
    {
        opts.push_back(OPTIONS[i]);
    }
}

std::string VFAFwdModel::GetDescription() const
{
    return "Calculates T1 from variable flip angle data, using the SPGR equation assuming a fixed TR and that TE<<T2*";
}

string VFAFwdModel::ModelVersion() const
{
    string version = "fwdmodel_vfa.cc";
#ifdef GIT_SHA1
    version += string(" Revision ") + GIT_SHA1;
#endif
#ifdef GIT_DATE
    version += string(" Last commit ") + GIT_DATE;
#endif
    return version;
}

void VFAFwdModel::Initialize(FabberRunData &rundata)
{
    m_TR = rundata.GetDouble("tr");
    string FAs_file = rundata.GetStringDefault("fas-file", "");
    if (FAs_file != "")
    {
        m_FAs = fabber::read_matrix_file(FAs_file);
    }
    vector<double> FAs = rundata.GetDoubleList("fa");
    if (FAs.size() > 0)
    {
        if (FAs_file != "")
        {
            throw FabberRunDataError("Can't specify flip angles in a file and also using individual options");
        }
        m_FAs.ReSize(FAs.size());
        for (unsigned int i = 0; i < FAs.size(); i++)
        {
            m_FAs(i + 1) = FAs[i];
        }
    }

    if (m_FAs.size() == 0) {
        throw InvalidOptionValue("fa<n>", "No flip angles given", "At least one flip angle must be specified");
    }

    if (!rundata.GetBool("radians"))
    {
        m_FAs *= M_PI / 180;
    }
}

void VFAFwdModel::NameParams(vector<string> &names) const
{
    names.clear();

    names.push_back("T1");
    names.push_back("sig0");
    names.push_back("B1corr");
}

void VFAFwdModel::HardcodedInitialDists(MVNDist &prior, MVNDist &posterior) const
{
    assert(prior.means.Nrows() == NumParams());

    SymmetricMatrix precisions = IdentityMatrix(NumParams()) * 1e-12;

    // Set priors
    prior.means(T1_index()) = 0.01;
    precisions(T1_index(), T1_index()) = 1e-12;

    prior.means(sig0_index()) = 0.01;
    precisions(sig0_index(), sig0_index()) = 1e-13;

    prior.means(B1corr_index()) = 1;
    precisions(B1corr_index(), B1corr_index()) = 1e99;

    // Set precsions on priors
    prior.SetPrecisions(precisions);

    // Set initial posterior
    posterior = prior;

    // For parameters with uniformative prior choose more sensible inital posterior
    // Tissue perfusion
    // posterior.means(T1_index()) = 1;
    // precisions(T1_index(), T1_index()) = 0.1;

    // posterior.SetPrecisions(precisions);
}

void VFAFwdModel::InitParams(MVNDist &posterior) const
{
    if (data.Nrows() != m_FAs.Nrows())
    {
        throw FabberRunDataError("Number of volumes in data: " + stringify(data.Nrows()) + " "
                                 "does not match number of flip angles: " + stringify(m_FAs.Nrows()));
    }

    // load the existing precisions as the basis for any update
    SymmetricMatrix precisions;
    precisions = posterior.GetPrecisions();

    // init the Sig0 Value - by setting T1 = 1 and finding the inverse for FA(1) assuming B1corr = 1
    double sig0;
    sig0 = data(1) * (1 - cos(m_FAs(1)) * exp(-m_TR)) / (sin(m_FAs(1) * (1 - exp(-m_TR))));
    posterior.means(sig0_index()) = sig0; // Approximate increase for a particular data point
    precisions(sig0_index(), sig0_index()) = 10;

    posterior.means(T1_index()) = 1;
    precisions(T1_index(), T1_index()) = 0.1;

    posterior.SetPrecisions(precisions);
}

void VFAFwdModel::Evaluate(const ColumnVector &params, ColumnVector &result) const
{
    // ensure that values are reasonable
    // negative check
    ColumnVector paramcpy = params;
    for (int i = 1; i <= NumParams(); i++)
    {
        if (params(i) < 0)
        {
            paramcpy(i) = 0;
        }
    }

    // parameters that are inferred - extract and give sensible names
    double sig0 = paramcpy(sig0_index());
    double T1 = paramcpy(T1_index());
    double B1corr = paramcpy(B1corr_index());
    if (T1 < 1e-8)
        T1 = 1e-8;
    if (sig0 < 1e-8)
        sig0 = 1e-8;

    int ntpts = m_FAs.Nrows();

    // --- SPGR Function ----
    ColumnVector sig(ntpts);
    sig = 0.0;
    for (int i = 1; i <= ntpts; i++)
    {
        sig(i) = sig0 * sin(m_FAs(i) * B1corr) * (1 - exp(-m_TR / T1)) / (1 - cos(m_FAs(i) * B1corr) * exp(-m_TR / T1));
    }
    result = sig;

    for (int i = 1; i <= ntpts; i++)
    {
        if (isnan(result(i)) || isinf(result(i)))
        {
            exit(0);
            LOG << "Warning NaN of inf in result" << endl;
            LOG << "result: " << result.t() << endl;
            LOG << "params: " << params.t() << endl;

            result = 0.0;
            break;
        }
    }
}

FwdModel *VFAFwdModel::NewInstance()
{
    return new VFAFwdModel();
}
