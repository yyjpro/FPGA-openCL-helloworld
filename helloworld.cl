__kernel void helloworld(__global int* restrict a, __global int* restrict b, __global int* restrict c)
{
    int i = 0;
    for(i=0;i<10;i++)
         {
            c[i] = a[i] + b[i];
         }
}