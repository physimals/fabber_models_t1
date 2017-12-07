/* dualecho_models.cc Shared library functions for dualecho models

Copyright (C) 2010-2011 University of Oxford */

/* CCOPYRIGHT  */

#include "t1_models.h"
#include "fwdmodel_vfa.h"

extern "C" {
int CALL get_num_models()
{
    return 1;
}

const char *CALL get_model_name(int index)
{
    switch (index)
    {
    case 0:
        return "vfa";
        break;
    default:
        return NULL;
    }
}

NewInstanceFptr CALL get_new_instance_func(const char *name)
{
    if (string(name) == "vfa")
    {
        return VFAFwdModel::NewInstance;
    }
    else
    {
        return NULL;
    }
}
}
