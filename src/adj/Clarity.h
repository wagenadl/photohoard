// Clarity.h

#ifndef CLARITY_H

#define CLARITY_H

bool zuiderveld_clahe(unsigned short *image,
                      int xblocksize, int xblockcount,
                      int yblocksize, int yblockcount, 
                      int nbins, double cliplimit);
/* Returns true if OK. */
/* Typically, nbins should be 4096. cliplimit should be >=1.
   blockcounts must be at least 2 and no more than 16.
*/

#endif
