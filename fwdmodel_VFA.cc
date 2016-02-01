/*  fwdmodel_VFA.cc - Implements T1 estimation using the Variable Flip Angle approach

    Jesper Kallehauge, IBME

    Copyright (C) 2016 University of Oxford  */

/*  CCOPYRIGHT */

#include "fwdmodel_VFA.h"

#include <iostream>
#include <newmatio.h>
#include <stdexcept>
#include "newimage/newimageall.h"
using namespace NEWIMAGE;
#include "fabbercore/easylog.h"
#include "miscmaths/miscprob.h"

FactoryRegistration<FwdModelFactory, VFAFwdModel>
  VFAFwdModel::registration("VFA");

string VFAFwdModel::ModelVersion() const
{
  return "$Id: fwdmodel_VFA.cc,v 1.11 2016/01/06 15:20:47 Kallehauge Exp $";
}

void VFAFwdModel::HardcodedInitialDists(MVNDist& prior,
    MVNDist& posterior) const
{
    Tracer_Plus tr("VFAFwdModel::HardcodedInitialDists");
    assert(prior.means.Nrows() == NumParams());

     SymmetricMatrix precisions = IdentityMatrix(NumParams()) * 1e-12;

    // Set priors
    // Fp or Ktrans whatever you belive
     prior.means(T1_index()) = 0.01;
     precisions(T1_index(),T1_index()) = 1e-12;

     prior.means(sig0_index()) = 0.01;
     precisions(sig0_index(),sig0_index()) = 1e-12;

    // Set precsions on priors
    prior.SetPrecisions(precisions);

    // Set initial posterior
    posterior = prior;

    // For parameters with uniformative prior chosoe more sensible inital posterior
    // Tissue perfusion
    posterior.means(T1_index()) = 1;
    precisions(T1_index(),T1_index()) = 0.1;

    posterior.SetPrecisions(precisions);

}



void VFAFwdModel::Evaluate(const ColumnVector& params, ColumnVector& result) const
{
  Tracer_Plus tr("VFAFwdModel::Evaluate");

    // ensure that values are reasonable
    // negative check
   ColumnVector paramcpy = params;
    for (int i=1;i<=NumParams();i++) {
      if (params(i)<0) { paramcpy(i) = 0; }
      }

   // parameters that are inferred - extract and give sensible names
   float T1;
   float sig0; //'inital' value of the signal
   ColumnVector FA_radians;

   // extract values from params
   sig0 = paramcpy(sig0_index());
   T1 = paramcpy(T1_index());
   if (T1<1e-8) T1=1e-8;
   if (sig0<1e-8) sig0=1e-8;

   ColumnVector FAvalshere; // the arterial signal to use for the analysis
   if (FAvals.Nrows()>0) {
     FAvalshere = FAvals; //use the artsig that was loaded by the model
   }
   else {
     //use an artsig from supplementary data
     if (suppdata.Nrows()>0) {
       FAvalshere = suppdata;
     }
     else {
       cout << "No valid b values found" << endl;
       throw;
     }
   }
   // use length of the aif to determine the number of time points
   int ntpts = FAvalshere.Nrows();
   FA_radians=FAvals*3.1415926/180;



   // --- SPGR Function ----
   ColumnVector sig(ntpts);
   sig=0.0;

   if (radians){
       for (int i=1; i<=ntpts; i++){
       sig(i) = sig0*sin(FAvals(i))*(1-exp(-TR/T1))/(1-cos(FAvals(i))*exp(-TR/T1));
       }
   }
       else{
       for (int i=1; i<=ntpts; i++){
       sig(i) = sig0*sin(FA_radians(i))*(1-exp(-TR/T1))/(1-cos(FA_radians(i))*exp(-TR/T1));
       }
   }

   result=sig;

   for (int i=1; i<=ntpts; i++) {
     if (isnan(result(i)) || isinf(result(i))) {
       exit(0);
       LOG << "Warning NaN of inf in result" << endl;
       LOG << "result: " << result.t() << endl;
       LOG << "params: " << params.t() << endl;

       result=0.0;
       break;
         }
   }


}

FwdModel* VFAFwdModel::NewInstance()
{
  return new VFAFwdModel();
}

void VFAFwdModel::Initialize(ArgsType& args)
{
  Tracer_Plus tr("VFAFwdModel::VFAFwdModel");
    string scanParams = args.ReadWithDefault("scan-params","cmdline");

    if (scanParams == "cmdline")
    {
      TR = convertTo<double>(args.Read("TR"));
      // specify command line parameters here
      //ColumnVector FAvals;
      string FAvalfile = args.Read("FAvals");
    //cout<<bvalfile<<endl;
      if (FAvalfile != "none") {
        FAvals = read_ascii_matrix( FAvalfile );
      }
    //cout<<bvalfile<<endl;
       radians = args.ReadBool("radians");
      doard=false;
     // if (inferart) doard=true;

      //imageprior = args.ReadBool("imageprior")
      // add information about the parameters to the log
      /* do logging here*/
    }

    else
        throw invalid_argument("Only --scan-params=cmdline is accepted at the moment");
}

vector<string> VFAFwdModel::GetUsage() const
{
  vector<string> usage;

  usage.push_back( "\nThis model is a one compartment model\n");
  usage.push_back( "It returns  2 parameters :\n");
  usage.push_back( " T1: the Apparent Diffusion Coefficient\n");
  usage.push_back( " sig0: the initial signal\n");

  return usage;
}

void VFAFwdModel::DumpParameters(const ColumnVector& vec,
                                    const string& indent) const
{

}

void VFAFwdModel::NameParams(vector<string>& names) const
{
  names.clear();

  names.push_back("T1");
  names.push_back("sig0");
}