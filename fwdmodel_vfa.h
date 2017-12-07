/*  fwdmodel_VFA.h - Implements the GRASE model

    Jesper Kallehauge, IBME

    Copyright (C) 2007-2016 University of Oxford  */

/*  CCOPYRIGHT */

#include "fabber_core/fwdmodel.h"

#include <newmat.h>

#include <string>
#include <vector>

class VFAFwdModel : public FwdModel {
public:
  static FwdModel* NewInstance();

  virtual void Initialize(FabberRunData &rundata);
  virtual std::string ModelVersion() const;
  virtual void GetOptions(std::vector<OptionSpec> &opts) const;
  virtual std::string GetDescription() const;

  virtual void NameParams(std::vector<std::string>& names) const;
  virtual int NumParams() const { return 2; }

  virtual void HardcodedInitialDists(MVNDist& prior, MVNDist& posterior) const;
  virtual void Evaluate(const NEWMAT::ColumnVector& params, NEWMAT::ColumnVector& result) const;

  virtual ~VFAFwdModel() {}

protected:
  // Lookup the starting indices of the parameters
  int T1_index() const {return 1;}
  int sig0_index() const { return 2; }

  double m_tr;
  bool m_radians;
  NEWMAT::ColumnVector m_fas;

private:
  /** Auto-register with forward model factory. */
  static FactoryRegistration<FwdModelFactory, VFAFwdModel> registration;
};