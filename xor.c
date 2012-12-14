
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char *argv[])
{
    FILE *fpin, *fpout;
    unsigned char val;
    
    if(argc != 3)
    {
        printf ("The argc is no equal 3\n");
        return -1;
    }
    
    fpin = fopen(argv[1], "rb");
    fpout = fopen(argv[2], "wb");
    if(fpin == NULL || fpout == NULL)
    {
        printf ("Failed to open file!!!\n");
        return -1;
    }
    do
    {
        if(fread(&val, 1, 1, fpin) != 1)
            break;
        val ^= 0x55;
        fwrite(&val, 1, 1, fpout);
    }while(1);
    fclose(fpin);
    fclose(fpout);
    return 0;
}
