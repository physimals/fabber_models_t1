# T1 Mapping in FABBER #

# Introduction

This project includes different ways of estimating T1 in the FABBER framework

Current models included:
1. The Variable Flip Angle approach ("VFA")
2. The Inversion Recovery approach ("IR")

# Running Model:

## VFA Command line example:

    fabber_t1
      --output=Data_out
      --data=Raw_T1_Data.nii 
      --mask=Raw_T1_Data_mask.nii 
      --method=vb 
      --model=vfa 
      --noise=white 
      --data-order=singlefile 
      --save-model-fit 
      --FAs=FAvals.txt      // Degrees
      --TR=0.2              // Seconds


## IR Command line example:

    fabber_t1
      --output=Data_out
      --data=Raw_T1_Data.nii 
      --mask=Raw_T1_Data_mask.nii 
      --method=vb 
      --model=IR 
      --noise=white 
      --data-order=singlefile 
      --save-model-fit 
      --TIs-file=TIs.txt      // Seconds
      --InvEfficiency         // Optional Boolean for Inversion Pulse Efficiency
