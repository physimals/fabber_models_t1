/*  fwdmodel_VFA.cc - Implements T1 estimation using the Variable Flip Angle approach

    Jesper Kallehauge, IBME

    Copyright (C) 2016 University of Oxford  */

/*  CCOPYRIGHT */

#include "fwdmodel_vfa.h"

#include "fabber_core/tools.h"

#include <newmatio.h>

#include <string>
#include <vector>

using namespace NEWMAT;
using namespace std;

FactoryRegistration<FwdModelFactory, VFAFwdModel> VFAFwdModel::registration("vfa");

static OptionSpec OPTIONS[] = {
    { "tr", OPT_FLOAT, "TR for VFA images", OPT_REQ, "" },
    { "fas-file", OPT_MATRIX, "File containing a list of flip angles", OPT_NONREQ, "" },
    { "fa<n>", OPT_MATRIX,
        "Alternative to fas-file, specify a sequence of flip angles --fa1=30 --fa2=38 etc",
        OPT_NONREQ, "" },
    { "radians", OPT_BOOL, "If specified, flip angles are given in radians", OPT_NONREQ, "" },
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
    return "Calculates T1 from variable flip angle data, using the SPGR equation assuming a fixed "
           "TR and that TE<<T2*";
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
    m_tr = rundata.GetDouble("tr");
    string fas_file = rundata.GetStringDefault("fas-file", "");
    if (fas_file != "")
    {
        m_fas = fabber::read_matrix_file(fas_file);
    }
    vector<double> fas = rundata.GetDoubleList("fa");
    if (fas.size() > 0)
    {
        if (fas_file != "")
        {
            throw FabberRunDataError(
                "Can't specify flip angles in a file and also using individual options");
        }
        m_fas.ReSize(fas.size());
        for (unsigned int i = 0; i < fas.size(); i++)
        {
            m_fas(i + 1) = fas[i];
        }
    }

    if (rundata.GetBool("radians")) 
    {
        m_fas = m_fas * 3.1415926 / 180;
    } 
}

void VFAFwdModel::NameParams(vector<string> &names) const
{
    names.clear();

    names.push_back("T1");
    names.push_back("sig0");
}

void VFAFwdModel::HardcodedInitialDists(MVNDist &prior, MVNDist &posterior) const
{
    assert(prior.means.Nrows() == NumParams());

    SymmetricMatrix precisions = IdentityMatrix(NumParams()) * 1e-12;

    // Set priors
    // Fp or Ktrans whatever you belive
    prior.means(T1_index()) = 0.01;
    precisions(T1_index(), T1_index()) = 1e-12;

    prior.means(sig0_index()) = 0.01;
    precisions(sig0_index(), sig0_index()) = 1e-12;

    // Set precsions on priors
    prior.SetPrecisions(precisions);

    // Set initial posterior
    posterior = prior;

    // For parameters with uniformative prior chosoe more sensible inital posterior
    // Tissue perfusion
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
    float T1;
    float sig0; //'inital' value of the signal
    ColumnVector FA_radians;

    // extract values from params
    sig0 = paramcpy(sig0_index());
    T1 = paramcpy(T1_index());
    if (T1 < 1e-8)
        T1 = 1e-8;
    if (sig0 < 1e-8)
        sig0 = 1e-8;

   
    int ntpts = m_fas.Nrows();
    if (data.Nrows() != ntpts) {
        throw FabberRunDataError("Number of volumes in data does not match number of flip angles");
    }
    
    // --- SPGR Function ----
    ColumnVector sig(ntpts);
    sig = 0.0;
    for (int i = 1; i <= ntpts; i++)
    {
        sig(i) = sig0 * sin(m_fas(i)) * (1 - exp(-m_tr / T1))
            / (1 - cos(m_fas(i)) * exp(-m_tr / T1));
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
