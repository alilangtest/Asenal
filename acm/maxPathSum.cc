#include <iostream> 
#include <cstdio> 
#include <algorithm> 
#include <cstring> 
#include <string>
#include <cmath> 
#include <vector> 
#include <queue> 
#include <map>
#include <ctime>
#include <set>

typedef long long lld; 
using namespace std; 
#ifdef DEBUG
#define debug(x) cout<<__LINE__<<" "<<#x<<"="<<x<<endl;
#else
#define debug(x)
#endif

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(NULL), right(NULL) {}
};

class Solution
{
#ifdef DEBUG
#define debug(x) cout<<__LINE__<<" "<<#x<<"="<<x<<endl;
#else
#define debug(x)
#endif
#define here cout<<__LINE__<< "  " << "_______________here" <<endl;
#define clr(NAME,VALUE) memset(NAME,VALUE,sizeof(NAME)) 
#define MAX 0x7fffffff 
#define MIN 0x80000000
#define N 16010
#define PRIME 999983
public:
    int f = 0, s = 0;
    void dfs(TreeNode *t, int now)
    {
        if (t == NULL) {
            return;
        }
        if (t->left == NULL && t->right == NULL) {
            if (now + t->val > f) {
                s = f;
                f = now + t->val;
            } else if (now + t->val > s) {
                s = now + t->val;
            }
        }
        dfs(t->left, now + t->val);
        dfs(t->right, now + t->val);

    }
    int maxPathSum(TreeNode *root)
    {
        dfs(root, 0);
        return f + s;
    }

};

int main()
{
    #ifdef DEBUG
    freopen("a", "r", stdin);
    #endif
    printf("%d %d\n", rand(), rand());
    Solution s;

    return 0;
}

