// ABC025 C - Two-Color Cards (Double-faced Cards)
// https://atcoder.jp/contests/abc025/tasks/abc025_c
// 3x3 grid game: two players alternately place B/W.
// Score = sum of |b[i][j]-b[i+1][j]|*c_ij for adjacent pairs.
// Player B maximizes, player W minimizes. Minimax with bitmask. O(9! but prunable)
#include <bits/stdc++.h>
using namespace std;
int b[3][3],c[3][3];
int B2[2][3],C2[3][2]; // vertical and horizontal bonuses
// Actually the problem: 3x3 board, b[i][j], c[i][j] given.
// Players place B(=1) or W(=0). Score = sum over adjacent same-color pairs.
// Let me re-read: score computed from b and c arrays for row/col adjacency.
// b[i][j]: bonus for vertical pair (i,j)-(i+1,j) being same color
// c[i][j]: bonus for horizontal pair (i,j)-(i,j+1) being same color
int bv[2][3],bh[3][2]; // vertical bonuses, horizontal bonuses
int board[3][3];
int calc_score(){
    int s=0;
    for(int i=0;i<2;i++) for(int j=0;j<3;j++)
        if(board[i][j]==board[i+1][j]) s+=bv[i][j];
    for(int i=0;i<3;i++) for(int j=0;j<2;j++)
        if(board[i][j]==board[i][j+1]) s+=bh[i][j];
    return s;
}
map<pair<int,int>,int> memo; // (mask of placed, color_mask) -> value
// turn: 0..8, even=player1(max), odd=player2(min)
// mask: which cells are filled
// color_mask: bit i = color of cell i (B=1,W=0)
int solve(int turn, int mask, int color_mask){
    if(turn==9){
        for(int i=0;i<3;i++) for(int j=0;j<3;j++)
            board[i][j]=(color_mask>>(i*3+j))&1;
        return calc_score();
    }
    auto key=make_pair(mask,color_mask);
    if(memo.count(key)) return memo[key];
    int res=(turn%2==0)?-1e9:1e9;
    for(int pos=0;pos<9;pos++){
        if(mask>>pos&1) continue;
        int nmask=mask|(1<<pos);
        int color=(turn%2==0)?1:0; // player1=B(1), player2=W(0)
        int ncmask=color_mask|(color<<pos);
        int val=solve(turn+1,nmask,ncmask);
        if(turn%2==0) res=max(res,val);
        else res=min(res,val);
    }
    return memo[key]=res;
}
int main(){
    for(int i=0;i<2;i++) for(int j=0;j<3;j++) cin>>bv[i][j];
    for(int i=0;i<3;i++) for(int j=0;j<2;j++) cin>>bh[i][j];
    cout<<solve(0,0,0)<<endl;
}
