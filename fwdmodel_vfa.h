/*  fwdmodel_VFA.h - Implements the GRASE model

    Jesper Kallehauge, IBME

    Modified by: Alex Smith, FMRIB, 20171206

    Copyright (C) 2007-2017 University of Oxford  */

/*  CCOPYRIGHT */

#include "fabber_core/fwdmodel.h"

#include "armawrap/newmat.h"

#include <string>
#include <vector>

class VFAFwdModel : public FwdModel
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
        return 3;
    }

    virtual void HardcodedInitialDists(MVNDist &prior, MVNDist &posterior) const;
    void InitParams(MVNDist &posterior) const;
    virtual void Evaluate(const NEWMAT::ColumnVector &params, NEWMAT::ColumnVector &result) const;

    virtual ~VFAFwdModel()
    {
    }

protected:
    // Lookup the starting indices of the parameters
    int T1_index() const
    {
        return 1;
    }
    int sig0_index() const
    {
        return 2;
    }
    int B1corr_index() const
    {
        return 3;
    }

    double m_TR;
    bool m_radians;
    NEWMAT::ColumnVector m_FAs;

private:
    /** Auto-register with forward model factory. */
    static FactoryRegistration<FwdModelFactory, VFAFwdModel> registration;
};
