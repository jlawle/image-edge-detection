# Optimized character recognition using edge detection
This program implements the same idea as my optical character recognition program. This program's implementation improves upon the last such that it utilizes edge detection for further determining correct letters of interest.

    This program utilizes thinning of each detected letter to analyze characteristics in verifying correct letter detection
    -  endpoints and branchpoints of a letter are counted
    -  Optimizes character recognition validating against ground truths

Below shows the results when verified against a ground truth file. A binary image is outputted from thresholding the matched spatial filter. The optimal threshold is found to be around 200-210 (chosing the closest distance to 1.0). 
- Can visually see the ratio of false positives (FPR) is significantly low compared to our ratio of true positives (TPR).

![ROC][roc.png]