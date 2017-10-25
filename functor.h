#ifndef FUNCTOR_H
#define FUNCTOR_H

class Functor
{
public:
	virtual void call() = 0;
};

template<class C>
class FunctorBase : public Functor
{
public:
        FunctorBase()
        {
                object() = 0;
        }

        FunctorBase(C* obj)
        {
                object() = obj;
        }

        void operator()()
        {
                call();
        }
private:
	MemberPtr(C*, object);
};

template<class C, void (C::*F)()>
class Functor0 :  public FunctorBase<C>
{
public:
        Functor0() : FunctorBase<C>()
        {
        }

        Functor0(C* obj) : FunctorBase<C>(obj)
        {
        }

        void call()
        {
                (this->object()->*F)();
        }
private:
};

template<class C, class P1, void (C::*F)(P1)>
class Functor1 :  public FunctorBase<C>
{
public:
        Functor1() : FunctorBase<C>()
        {
        }

        Functor1(C* obj, P1 arg1) : FunctorBase<C>(obj)
        {
		p1() = arg1;
        }

        void call()
        {
                (this->object()->*F)(p1());
        }
private:
	Member(P1, p1);
};

template<class C, class P1, class P2, void (C::*F)(P1,P2)>
class Functor2 :  public FunctorBase<C>
{
public:
        Functor2() : FunctorBase<C>()
        {
        }

        Functor2(C* obj, P1 arg1, P2 arg2) : FunctorBase<C>(obj)
        {
		p1() = arg1;
		p2() = arg2;
        }

        void call()
        {
                (this->object()->*F)(p1(), p2());
        }
private:
        Member(P1, p1);
	Member(P2, p2);
};

#endif

