This project includes different ways of estimating T1 in the FABBER framework

Currently one model is included. 
    1. The Variable Flip Angle approach ("VFA")

Command line example

--output=/home/fsl/Desktop/Data_out/Data 
--data=/home/fsl/Desktop/Data/FA_raw.nii 
--mask=/home/fsl/Desktop/Data/FA_raw_mask.nii 
--method=vb 
--model=VFA 
--noise=white 
--data-order=singlefile 
--save-model-fit 
--FAvals=/home/fsl/Desktop/Data/FAvals.dat 
--TR=0.00343