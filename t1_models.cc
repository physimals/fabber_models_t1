/* t1_models.cc Measured T1 solver for FABBER

Copyright (C) 2010-2017 University of Oxford */

/* CCOPYRIGHT  */

#include "t1_models.h"
#include "fwdmodel_IR.h"
#include "fwdmodel_vfa.h"

#include <string>

extern "C" {
int CALL get_num_models()
{
    return 2;
}

const char *CALL get_model_name(int index)
{
    switch (index)
    {
    case 0:
        return "vfa";
        break;
    case 1:
        return "ir";
        break;
    default:
        return NULL;
    }
}

NewInstanceFptr CALL get_new_instance_func(const char *name)
{
    if (std::string(name) == "vfa")
    {
        return VFAFwdModel::NewInstance;
    }
    else if (std::string(name) == "ir")
    {
        return IRFwdModel::NewInstance;
    }
    else
    {
        return NULL;
    }
}
}
