//#include <string>

struct s{
	int x;
};

class C
{
public:
	int x;
	int f(int x);
private:
	int y;
};

int C::f(int x){
	int result = 10;
	result+=x;
	int *p=&result;
	return result;
}

int g(int x,
      int y){
	int a;
	a = x+y;
	return a;
}

