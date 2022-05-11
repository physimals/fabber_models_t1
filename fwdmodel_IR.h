/*  fwdmodel_IR.h - Implements an IR T1 solver in FABBER

    Alex Smith, FMRIB

    Copyright (C) 2007-2016 University of Oxford  */

/*  CCOPYRIGHT */

#include "fabber_core/fwdmodel.h"

#include "armawrap/newmat.h"

#include <string>
#include <vector>

class IRFwdModel : public FwdModel
{
public:
    static FwdModel *NewInstance();

    virtual void Initialize(FabberRunData &rundata);
    virtual std::string ModelVersion() const;
    virtual void GetOptions(std::vector<OptionSpec> &opts) const;
    virtual std::string GetDescription() const;

    virtual void NameParams(std::vector<std::string> &names) const;
    virtual int NumParams() const
    {
        return (2 + (m_InvEff ? 1 : 0));
    }

    virtual void HardcodedInitialDists(MVNDist &prior, MVNDist &posterior) const;
    virtual void Evaluate(const NEWMAT::ColumnVector &params, NEWMAT::ColumnVector &result) const;

    virtual ~IRFwdModel()
    {
    }

protected:
    NEWMAT::ColumnVector m_TI;
    bool m_InvEff;

private:
    /** Auto-register with forward model factory. */
    static FactoryRegistration<FwdModelFactory, IRFwdModel> registration;
};
