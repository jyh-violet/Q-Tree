//
// Created by workshop on 9/1/2021.
//

#include <math.h>
#include <stdio.h>
#include <papi.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include "Tools.h"
#include "common.h"
#define N 999
/*
int zipf(double alpha, int n)
{
    static int first = 1;      // Static first time flag
    static double c = 0;          // Normalization constant
    static double *sum_probs;     // Pre-calculated sum of probabilities
    double z;                     // Uniform random number (0 < z < 1)
    int zipf_value;               // Computed exponential value to be returned
    int    i;                     // Loop counter
    int low, high, mid;           // Binary-search bounds

    // Compute normalization constant on first call only
    if (first == 1)
    {
        for (i=1; i<=n; i++)
            c = c + (1.0 / pow((double) i, alpha));
        c = 1.0 / c;

        sum_probs = (double *)(malloc((n + 1) * sizeof(*sum_probs)));
        sum_probs[0] = 0;
        for (i=1; i<=n; i++) {
            sum_probs[i] = sum_probs[i-1] + c / pow((double) i, alpha);
        }
        first = 0;
    }

    // Pull a uniform random number (0 < z < 1)
    do
    {
        z = (rand() % (N + 1) )/ (double)(N + 1);
    }
    while ((z == 0) || (z == 1));

    // Map z to the value
    low = 1, high = n, mid;
    do {
        mid = floor((low+high)/2);
        if (sum_probs[mid] >= z && sum_probs[mid-1] < z) {
            zipf_value = mid;
            break;
        } else if (sum_probs[mid] >= z) {
            high = mid-1;
        } else {
            low = mid+1;
        }
    } while (low <= high);


    return(zipf_value);
}
 */

/**
   * Number of items.
   */
 long items;

/**
 * Min item to generate.
 */
 long base;

/**
 * The zipfian constant to use.
 */
double zipfianconstant;

/**
 * Computed parameters for generating the distribution.
 */
 double alpha, zetan, eta, theta, zeta2theta;

/**
 * The number of items used to compute zetan the last time.
 */
 long countforzeta;

 void initZipfParameter(int n, double zipfianpara){
     items = n;
     base = 0;
     zipfianconstant = zipfianpara;
     theta = zipfianconstant;
     zeta2theta = zeta(2, theta);
     alpha = 1.0 / (1.0 - theta);
     zetan = zetastatic(n, zipfianpara);
     countforzeta = items;
     eta = (1 - pow(2.0 / items, 1 - theta)) / (1 - zeta2theta / zetan);
 }

/**
 * Compute the zeta constant needed for the distribution. Do this from scratch for a distribution with n items,
 * using the zipfian constant theta. This is a static version of the function which will not remember n.
 * @param n The number of items to compute zeta over.
 * @param theta The zipfian constant.
 */
 double zetastatic(long n, double theta) {
    double sum = 0;
    for (long i = 0; i < n; i++) {

        sum += 1 / (pow(i + 1, theta));
    }

    return sum;
}
/**
 * Compute the zeta constant needed for the distribution. Do this from scratch for a distribution with n items,
 * using the zipfian constant thetaVal. Remember the value of n, so if we change the itemcount, we can recompute zeta.
 *
 * @param n The number of items to compute zeta over.
 * @param thetaVal The zipfian constant.
 */
double zeta(long n, double thetaVal) {
    return zetastatic(n, thetaVal);
}




// from ycsb
int zipf(){
    double u = ((double )rand()) / ((double )RAND_MAX + 1);
    double uz = u * zetan;

    if (uz < 1.0) {
        return base;
    }

    if (uz < 1.0 + pow(0.5, theta)) {
        return base + 1;
    }

    int ret =  (int) ((items) * pow(eta * u - eta + 1, alpha));
    return ret;
}


void printArray(int* array, int num){

    for(int i = 0; i < num; i ++){
        printf("%d,",  *(array + i));
    }
    printf("\n");
}

void bubbleSort(void* arr[], int len, BOOL (*cmp)(void*, void *)) {
    int i, j;
    void *temp;
    for (i = 0; i < len - 1; i++)
        for (j = 0; j < len - 1 - i; j++)
            if (cmp(arr[j], arr[j + 1])) {
                temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
}

char *myItoa(int num, char *str){
    if(str == NULL)
    {
        return NULL;
    }
    sprintf(str, "%d", num);
    return str;
}


void InsertSort(int a[], int l, int r)
{
    for(int i = l + 1; i <= r; i++)
    {
        if(a[i - 1] > a[i])
        {
            int t = a[i];
            int j = i;
            while(j > l && a[j - 1] > t)
            {
                a[j] = a[j - 1];
                j--;
            }
            a[j] = t;
        }
    }
}

int FindMid(int a[], int l, int r)
{
    if(l == r) return l;
    int i = 0;
    int n = 0;
    for(i = l; i < r - 5; i += 5)
    {
        InsertSort(a, i, i + 4);
        n = i - l;
        int temp = a[l + n / 5];
        a[l + n / 5] = a[i + 2];
        a[i + 2] = temp;
    }

    //处理剩余元素
    int num = r - i + 1;
    if(num > 0)
    {
        InsertSort(a, i, i + num - 1);
        n = i - l;
        int temp = a[l + n / 5];
        a[l + n / 5] = a[i + num / 2];
        a[i + num / 2] = temp;
    }
    n /= 5;
    if(n == l) return l;
    return FindMid(a, l, l + n);
}

int Partion(int a[], int l, int r, int p)
{
    int temp = a[p];
    a[p] = a[l];
    a[l] = temp;
    int i = l;
    int j = r;
    int pivot = a[l];
    while(i < j)
    {
        while(a[j] >= pivot && i < j)
            j--;
        a[i] = a[j];
        while(a[i] <= pivot && i < j)
            i++;
        a[j] = a[i];
    }
    a[i] = pivot;
    return i;
}

int BFPRT(int a[], int l, int r, int k)
{
    int p = FindMid(a, l, r);    //寻找中位数的中位数
    int i = Partion(a, l, r, p);

    int m = i - l + 1;
    if(m == k) return a[i];
    if(m > k)  return BFPRT(a, l, i - 1, k);
    return BFPRT(a, i + 1, r, k - m);
}

int printLog;
int logF = -1;
__thread threadID;
void vmlog(LOGLevel logLevel, char* fmat, ...){
//    if(logLevel == InsertLog){
//        return;
//    }
    if(!printLog || logLevel < WARN){
        return;
    }
    //get the string passed by the caller through the format string
    va_list argptr;
    va_start(argptr, fmat);
    char buffer[MAX_LOG_SIZE]="\0";
    int count = vsprintf(buffer, fmat, argptr);
    va_end(argptr);
    if(count >= MAX_LOG_SIZE)
    {
        printf("ERROR:log message too many\n");
        return;
    }
//    printf("(pid:%lu)---%s\n", pthread_self(), buffer);
//    fflush(stdout);

    if(logF <= 0)
    {
        if (access(LOG_PATH,0) == 0 ){
            remove(LOG_PATH);
        }
        logF = open(LOG_PATH, O_WRONLY|O_CREAT, S_IRWXU  );
        lseek(logF, 0, SEEK_SET);
    }
    char output[MAX_LOG_SIZE + 100];
    snprintf(output, sizeof(output),"(pid:%lu)---%s\n",threadID, buffer);
    write(logF, output, strlen(output));
    if(logLevel == ERROR){
        exit(-1);
    }
}