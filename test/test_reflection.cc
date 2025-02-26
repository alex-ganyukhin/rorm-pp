#include <experimental/meta>

#include <iostream>

struct S
{
    unsigned i : 2, j : 6;
};

consteval auto member_number( int n )
{
    if ( n == 0 )
        return ^^S::i;
    else if ( n == 1 )
        return ^^S::j;
    throw 1;
}

int main()
{
    S s { 0, 0 };
    s.[:member_number( 1 ):] = 42; // Same as: s.j = 42;

    std::cout << s.i << "\n";
    std::cout << s.j << "\n";
}
