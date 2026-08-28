// Random Walk Expected Value - 1D with Absorbing Barriers
// Start at position s (0 < s < N). At each step, move left with prob q, right with prob p.
// Positions 0 and N are absorbing. Find expected steps to absorption.
// E[0]=E[N]=0, E[i] = 1 + p*E[i+1] + q*E[i-1]
// For p=q=0.5: E[i] = i*(N-i)
// General case: solve linear system. O(N)
#include <bits/stdc++.h>
using namespace std;
int main(){
    int N,s; cin>>N>>s;
    double p; cin>>p;
    double q=1-p;
    // E[0]=0, E[N]=0
    // E[i] = 1 + p*E[i+1] + q*E[i-1]
    // Rearrange: E[i] - p*E[i+1] - q*E[i-1] = 1
    // Since p+q=1: (p+q)*E[i] - p*E[i+1] - q*E[i-1] = 1
    //              p*(E[i]-E[i+1]) = q*(E[i-1]-E[i]) + 1
    // Let D[i] = E[i] - E[i+1]
    // p*D[i] = q*(-D[i-1]) + 1... tridiagonal solve is simpler:
    // Use Thomas algorithm for tridiagonal system
    // -q*E[i-1] + E[i] - p*E[i+1] = 1
    vector<double> E(N+1,0);
    // Forward sweep
    vector<double> c(N+1,0),d(N+1,0);
    // E[1] - p*E[2] = 1 + q*E[0] = 1
    c[1]=p; d[1]=1;
    for(int i=2;i<N;i++){
        double w=q/(1-q*c[i-1]);
        // this is wrong... let me just do Gaussian elimination on tridiag
        // a_i * E[i-1] + b_i * E[i] + c_i * E[i+1] = d_i
        // a_i = -q, b_i = 1, c_i = -p, d_i = 1
        double denom=1-(-q)*c[i-1]; // b_i - a_i * c'[i-1]
        c[i]=p/denom;    // -c_i / denom = p/denom
        d[i]=(1+q*d[i-1])/denom; // (d_i - a_i*d'[i-1])/denom
    }
    // back substitution
    // E[N-1] at boundary: no c term since E[N]=0
    double denom_last=1-(-q)*c[N-1]; // should just use the loop properly
    // Actually last row: -q*E[N-2] + E[N-1] = 1 (since E[N]=0)
    // Let me redo properly for i=1..N-1
    // Recurrence: E[i] = (1 + q*E[i-1] + p*E[i+1])
    // Thomas: forward
    vector<double> cp(N),dp2(N);
    // i=1: E[1] = 1 + q*0 + p*E[2] => E[1] - p*E[2] = 1
    cp[1]=p; dp2[1]=1;
    for(int i=2;i<N;i++){
        double m=q; // coeff of E[i-1]
        double den=1.0-m*cp[i-1];
        cp[i]=p/den;
        dp2[i]=(1.0+m*dp2[i-1])/den;
    }
    E[N]=0;
    E[N-1]=dp2[N-1]; // since cp[N-1]*E[N]=0
    for(int i=N-2;i>=1;i--)
        E[i]=dp2[i]+cp[i]*E[i+1];
    printf("%.9f\n",E[s]);
}
