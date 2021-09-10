
class C
{
public:
	int x;
	int f(int x);
private:
	int y;
};

int C::f(int x){
	int result = x + 10;
	return result;
}

int g(int x,
      int y){
	return x+y;
}

