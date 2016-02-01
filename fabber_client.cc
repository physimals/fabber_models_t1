/*  FABBER - Fast T1 Bayesian Estimation Routine

    Jesper Kallehauge, IBME

    Copyright (C) 2007-2016 University of Oxford  */

/*  CCOPYRIGHT */

#include "fabbercore/fabber_core.h"

// T1 models to be included from library
#include "fwdmodel_VFA.h"
int main(int argc, char** argv) {

  //add the T1 models - these will autoregister at this point
  VFAFwdModel::NewInstance();
  
  return execute(argc, argv);

}