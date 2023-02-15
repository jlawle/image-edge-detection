/*  John Lawler - ECE 6310 - Lab 3
 *  letter recognition with image thinning
 *
 *  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int MROWS, MCOLS, IROWS, ICOLS, TROWS, TCOLS, BYTES, i;
    char header[320];
    unsigned char *image, *msfImage;
    FILE *imageFile, *gtFile, *msfFile, *templateFile, *binaryFile;


    // read image file
    if ((imageFile = fopen("parenthood.ppm","rb")) == NULL) {
        printf("Unable to open parenthood.ppm for reading\n");
        exit(0);
      }
    fscanf(imageFile, "%s %d %d %d", header, &ICOLS, &IROWS, &BYTES);
    if (strcmp(header, "P5") != 0  ||  BYTES != 255) {
        printf("Input image is not an 8-bit image\n");
        exit(0);
      }

    image = (unsigned char*)calloc(IROWS*ICOLS, sizeof(unsigned char));
    fread(image, 1, IROWS*ICOLS, imageFile);
    fclose(imageFile);

    // read msf file
    if ((msfFile = fopen("msf_e.ppm","rb")) == NULL) {
        printf("Unable to open msf_e.ppm for reading\n");
        exit(0);
      }
    fscanf(msfFile, "%s %d %d %d", header, &MCOLS, &MROWS, &BYTES);
    if (strcmp(header, "P5") != 0  ||  BYTES != 255) {
        printf("MSF image is not an 8-bit image\n");
        exit(0);
      }

    msfImage = (unsigned char*)calloc(MROWS*MCOLS, sizeof(unsigned char));
    fread(msfImage, 1, MROWS*MCOLS, msfFile);
    fclose(msfFile);
    
    
    // read template file
    if ((templateFile = fopen("parenthood_e_template.ppm","rb")) == NULL) {
        printf("Unable to open parenthood.ppm for reading\n");
        exit(0);
      }
    fscanf(templateFile, "%s %d %d %d", header, &TCOLS, &TROWS, &BYTES);
    if (strcmp(header, "P5") != 0  ||  BYTES != 255) {
        printf("Template image is not an 8-bit image\n");
        exit(0);
      }

    // Ground truth file and verification
    int detected, gtRow, gtCol;
    char gt_letter;
    unsigned char *pixelImage, *binImage, *temp, *binTemp;

    int unthinned, tr, tc, c, r, endpt, branchpt;
    int TP, FP, TN, FN;
    int TH;

    // loop across range of thresholds
    for ( TH = 190; TH <= 230; TH++){
        // open ground truth file each time
        if ((gtFile = fopen("parenthood_gt.txt","r")) == NULL) {
            printf("Unable to open parenthood_gt.txt for reading\n");
            exit(0);
        }

        // reaclloate to zero all the images
        pixelImage = (unsigned char*)calloc(TCOLS*TROWS, sizeof(unsigned char));
        binImage = (unsigned char*)calloc(TCOLS*TROWS, sizeof(unsigned char));
        binTemp = (unsigned char*)calloc((TCOLS+2)*(TROWS+2), sizeof(unsigned  char));
        TP = FP = TN = FN = 0;
        endpt = branchpt = 0;

         // Loop through groud truth file until end and verify each location
          while (1) {
             i = fscanf(gtFile, "%s %d %d", &gt_letter, &gtCol, &gtRow);
             if (i != 3) break;                  
             detected = 0;

             // Search for a detected pixel at each letter
             for (tr = -7; tr <= 7; tr++)
                 for (tc = -4; tc <= 4; tc++) {
                    if(msfImage[ICOLS*(gtRow + tr)+(gtCol + tc)]  > TH){
                        detected = 1;
                     }
                 }

             gtCol += 1;
             if (detected == 1) {
                // Copy the gt location to pixelImage
                i = 0;
                for (tr = -7; tr <= 7; tr++)
                    for (tc = -4; tc <= 4; tc++) {
                     pixelImage[i] = image[ICOLS*(gtRow + tr)+(gtCol + tc)];
                     i++;
                    }

                // Threshold pixelImage at 128 to create binary
                for (i = 0; i < TROWS*TCOLS; i++)
                    if (pixelImage[i] > 128) binImage[i] = 255;
                    else binImage[i] = 0;

                int count, ncount, erase;

                binaryFile = fopen("BinImage.ppm", "wb");
                fprintf(binaryFile,"P5 %d %d 255\n", TCOLS, TROWS);
                fwrite(binImage, 1, TROWS*TCOLS, binaryFile);
                fclose(binaryFile);
                unthinned = 1;

                // set binTemp to all white
                for (i = 0; i < (TROWS+2)*(TCOLS+2); i++)
                    binTemp[i]=255;

                while(unthinned) {
                    temp = (unsigned char *)calloc(3*3, sizeof(unsigned char));
                    erase = 0;
                    
                    // set bin temp
                    for (r = 0; r < TROWS; r++)
                        for (c = 0; c < TCOLS; c++){
                            binTemp[(TCOLS+2)*(r+1) + (c+1)] = binImage[TCOLS*r + c];
                        }
    
                    // Loop for bounds of the original binImage inside the binTemp
                    for (r = 1; r <= TROWS; r++) {
                        for (c = 1; c <= TCOLS; c++) {
                            //printf("entering double For\n");
                            
                            // create temp array    
                            for (tr = -1; tr <= 1; tr++)
                                for (tc = -1; tc <= 1; tc++){
                                    temp[3*(tr+1) + (tc+1)] = binTemp[(TCOLS+2)*(r+tr) + (c+tc)];
                            }
                            temp[4] = 0;
                            count = 0;
                            ncount = 0;
                            
                            // Make sure that only checks black pixels
                            if ( binImage[TCOLS*(r-1) + (c-1)] == 0) {
                                // check transitions (only be 1 to count)
                                for ( i = 0; i < 2; i++)
                                    if (temp[i] == 0  && temp[i+1] == 255) count++;
                                if (temp[2] == 0 && temp[5] == 255) count++;
                                if (temp[5] == 0 && temp[8] == 255) count++;
                                for ( i = 8; i > 6; i--)
                                    if (temp[i] == 0 && temp[i-1] == 255) count++;
                                if (temp[6] == 0 && temp[3] == 255) count++;
                                if (temp[3] == 0 && temp[0] == 255) count++;
                                
                                // if only one edge -> non edge transition
                                if (count == 1) {
                                    // count neighbors must be 2 and 6 inclusive
                                    for (i = 0; i < 9; i++)
                                        if ( i != 4 && temp[i] == 0) ncount++;

                                    // Check N or E or (W and S)
                                    // N = 1, E = 5, W = 3, S = 7
                                    if (ncount >= 2 && ncount <= 6){
                                        if (temp[1] == 255 || temp[5] == 255 || 
                                            (temp[3] == 255 && temp[7] == 255)){
                                            erase++;
                                            binImage[TCOLS*(r-1) + (c-1)] = 255;
                                        } 
                                    } /* end of checking N E W S */
                                } /* end of if transition count = 1 */
                            }
                        } /* end of binTemp col loop */
                    } /* end of row loop */

                    /* if reach end of loop through template and no more pixels got erased*/
                    if (erase == 0) {
                        unthinned = 0;
                    }
                } /* end of while thinning */
 
                // Scan thinned image for branch and end points
                endpt = 0;
                branchpt = 0;
                for (r = 1; r <= TROWS; r++) {
                    for (c = 1; c <= TCOLS; c++) {
                        // create temp array    
                        for (tr = -1; tr <= 1; tr++)
                            for (tc = -1; tc <= 1; tc++){
                                temp[3*(tr+1) + (tc+1)] = binTemp[(TCOLS+2)*(r+tr) + (c+tc)];
                        }

                        // check transisitons
                        count = 0;
                        if ( binImage[TCOLS*(r-1) + (c-1)] == 0) {
                             for ( i = 0; i < 2; i++)
                                 if (temp[i] == 0  && temp[i+1] == 255) count++;
                             if (temp[2] == 0 && temp[5] == 255) count++;
                             if (temp[5] == 0 && temp[8] == 255) count++;
                             for ( i = 8; i > 6; i--)
                                 if (temp[i] == 0 && temp[i-1] == 255) count++;
                             if (temp[6] == 0 && temp[3] == 255) count++;
                             if (temp[3] == 0 && temp[0] == 255) count++;
                
                            // check count to be 1 or >2
                            if (count == 1) endpt++; 
                            else if (count > 2) branchpt++;
                        }
                    }
                }
            
                // Write binary image to file
                binaryFile = fopen("binOutputErased.ppm", "wb");
                fprintf(binaryFile,"P5 %d %d 255\n", TCOLS, TROWS);
                fwrite(binImage, 1, TROWS*TCOLS, binaryFile);
                fclose(binaryFile);
             } /* if detected */  

             // determine TP, FP, TN, FN
             if (detected == 1) {
                if (endpt == 1 && branchpt == 1 && gt_letter == 'e'){ TP++;
                }else if (endpt != 1 || branchpt != 1){ 
                    if (gt_letter != 'e') FP++;
                    else TP++;
                } else if (gt_letter != 'e') { FP++;
                } else TN++;


             } else if (gt_letter == 'e') { FN++;
             } else if (gt_letter != 'e') { TN++;
             }

         } /* while( gtloc)  */
        fclose(gtFile);       
        printf(" Threshold: %d TP: %d \t FP: %d \t TN: %d \t FN: %d\n", TH, TP, FP, TN, FN);
    } /* forloop threshold */
}
