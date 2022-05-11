/*  fwdmodel_IR.cc - Implements T1 estimation using the Variable Flip Angle approach

    Alex Smith, FMRIB

    Copyright (C) 2007-2017 University of Oxford  */

/*  CCOPYRIGHT */

#include "fwdmodel_IR.h"

#include "fabber_core/tools.h"

#include "armawrap/newmat.h"

#include <string>
#include <vector>

using namespace NEWMAT;
using namespace std;

FactoryRegistration<FwdModelFactory, IRFwdModel> IRFwdModel::registration("ir");

static OptionSpec OPTIONS[] = {
    { "tis-file", OPT_MATRIX, "File containing a list of inversion times (s)", OPT_REQ, "" },
    { "invefficiency", OPT_BOOL, "If specified, will also calculate the inversion pulse efficiency", OPT_NONREQ, "" },
    { "" }
};

void IRFwdModel::GetOptions(vector<OptionSpec> &opts) const
{
    for (int i = 0; OPTIONS[i].name != ""; i++)
    {
        opts.push_back(OPTIONS[i]);
    }
}

std::string IRFwdModel::GetDescription() const
{
    return "Calculates T1 from inversion recovery data, assuming the signal has fully relaxed between each image.";
}

string IRFwdModel::ModelVersion() const
{
    string version = "fwdmodel_IR.cc";
#ifdef GIT_SHA1
    version += string(" Revision ") + GIT_SHA1;
#endif
#ifdef GIT_DATE
    version += string(" Last commit ") + GIT_DATE;
#endif
    return version;
}

void IRFwdModel::Initialize(FabberRunData &rundata)
{
    string TIs = rundata.GetStringDefault("tis-file", "");
    m_TI = fabber::read_matrix_file(TIs);

    m_InvEff = false;
    m_InvEff = rundata.ReadBool("invefficiency");
}

void IRFwdModel::NameParams(vector<string> &names) const
{
    names.clear();

    names.push_back("T1");
    names.push_back("sig0");
    if (m_InvEff)
        names.push_back("InvEff");
}

void IRFwdModel::HardcodedInitialDists(MVNDist &prior, MVNDist &posterior) const
{
    assert(prior.means.Nrows() == NumParams());

    SymmetricMatrix precisions = IdentityMatrix(NumParams()) * 1e-12;

    int place = 1;
    // Set priors
    // T1
    prior.means(place) = 1;
    precisions(place, place) = 0.44; // Based off of similar prior precision in FABBER model
    ++place;

    // S0
    prior.means(place) = 0.01;
    precisions(place, place) = 1e-12;
    ++place;

    if (m_InvEff)
    {
        prior.means(place) = 1;
        precisions(place, place) = 1e99;
        ++place;
    }

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

void IRFwdModel::Evaluate(const ColumnVector &params, ColumnVector &result) const
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
    double T1;
    double sig0;   //'inital' value of the signal
    double InvEff; // B1 Correction Factor
    int place = 1;

    // extract values from params
    // T1
    T1 = paramcpy(place);
    ++place;

    sig0 = paramcpy(place);
    ++place;

    if (m_InvEff)
    {
        InvEff = paramcpy(place);
        ++place;
    }
    else
    {
        InvEff = 1;
    }

    // Keep sensible values for parameters
    if (T1 < 1e-8)
        T1 = 1e-8;
    if (sig0 < 1e-8)
        sig0 = 1e-8;

    int ntpts = m_TI.Nrows();
    if (data.Nrows() != ntpts)
    {
        throw FabberRunDataError("Number of volumes in data does not match number of inversion times");
    }

    // --- IR Function ----
    ColumnVector sig(ntpts);
    sig = 0.0;
    for (int i = 1; i <= ntpts; i++)
    {
        sig(i) = fabs(sig0 * (1 - 2 * InvEff * exp(-m_TI(i) / T1)));
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

FwdModel *IRFwdModel::NewInstance()
{
    return new IRFwdModel();
}
