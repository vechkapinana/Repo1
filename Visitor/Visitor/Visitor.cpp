// Visitor.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <cassert>
#include <string> 
using namespace std;

struct Transformer;
struct Number;
struct BinaryOperation;
struct FunctionCall;
struct Variable;


struct Expression  //базовая абстрактная структур
{
    virtual ~Expression() { }  //виртуальный деструктор
    virtual double evaluate() const = 0; //абстрактный метод «вычислить»
    virtual Expression* transform(Transformer* tr) const = 0; //абстрактный метод печать(копирование)
    virtual std::string print() const = 0;//абстрактный метод печать
};
struct Transformer //pattern Visitor
{
    virtual ~Transformer() { }
    virtual Expression* transformNumber(Number const*) = 0;
    virtual Expression* transformBinaryOperation(BinaryOperation const*) = 0;
    virtual Expression* transformFunctionCall(FunctionCall const*) = 0;
    virtual Expression* transformVariable(Variable const*) = 0;
};
struct Number : Expression // стуктура «Число»
{
    Number(double value) : value_(value) {};
    double value() const { return value_; };
    double evaluate() const { return value_; };
    ~Number() {}//деструктор, тоже виртуальный

    string print() const { return to_string(this->value_); }
    Expression* transform(Transformer* tr) const
    {
        return tr->transformNumber(this);
    }
private:
    double value_;
};


struct BinaryOperation : Expression // «Бинарная операция»
{
    enum {
        PLUS = '+',
        MINUS = '-',
        DIV = '/',
        MUL = '*'
    };

    BinaryOperation(Expression const* left, int op, Expression const* right) : left_(left), op_(op), right_(right) 
    {
        assert(left_ && right_);
    }
    ~BinaryOperation() //в деструкторе освободим занятую память 
    {
        delete left_;
        delete right_;
    }
    double evaluate() const 
    {
        double left = left_->evaluate(); // вычисляем левую часть
        double right = right_->evaluate(); // вычисляем правую часть
        switch (op_) // в зависимости от вида операции, складываем, вычитаем, умножаем
            // или делим левую и правую части
        {
        case PLUS: return left + right;
        case MINUS: return left - right;
        case DIV: return left / right;
        case MUL: return left * right;
        }
    };
    Expression* transform(Transformer* tr) const
    {
        return tr->transformBinaryOperation(this);
    };

    Expression const* left() const { return left_; } // чтение левого операнда;
    Expression const* right() const { return right_; } // чтение правого операнда;
    int operation() const { return op_; } // чтение символа операции;

    std::string print() const {
        return this->left_->print() + std::string(1, this->op_) + this->right_->print();
    }

private:
    Expression const* left_;
    Expression const* right_;
    int op_;
};


struct FunctionCall : Expression
{
    FunctionCall(std::string const& name, Expression const* arg) : name_(name), arg_(arg)
    {
        assert(arg_);
        assert(name_ == "sqrt" || name_ == "abs");
    } // разрешены только вызов sqrt и abs

    ~FunctionCall() { delete arg_; };

    double evaluate() const { // реализация виртуального метода «вычислить»
        if (name_ == "sqrt")
            return sqrt(arg_->evaluate()); // либо вычисляем корень квадратный
        else return fabs(arg_->evaluate()); // либо модуль — остальные функции запрещены
    } 

    Expression* transform(Transformer* tr) const { return tr->transformFunctionCall(this); };
    
    string const& name() const { return name_; };
    Expression const* arg() const { return arg_; };

    string print() const {
        return this->name_ + "(" + this->arg_->print() + ")";
    }
private:
    string const name_;
    Expression const* arg_;
};

//Макрос assert() обычно используется для проверки корректности работы программы.





struct Variable : Expression // структура «Переменная»
{
    Variable(string const& name) : name_(name) {};
    string const& name() const { return name_; } // чтение имени переменной;
    double evaluate() const { return 0.0; };

    std::string print() const { return this->name_; }

    Expression* transform(Transformer* tr) const { return tr->transformVariable(this); }
private:
    string const name_;
};

/*
*реализуйте все необходимые методы класса
* вы можете определять любые вспомогательные
* методы, если хотите
*/
struct CopySyntaxTree : Transformer
{
    Expression* transformNumber(Number const* number)
    {
        Expression* e = new Number(number->value());
        return e;
    }
    Expression* transformBinaryOperation(BinaryOperation const* op)
    {
        Expression* e = new BinaryOperation((op->left())->transform(this),
            op->operation(),
            (op->right())->transform(this));
        return e;
    }
    Expression* transformFunctionCall(FunctionCall const* fcall)
    {
        Expression* e = new FunctionCall(fcall->name(), (fcall->arg())->transform(this));
        return e;
    }
    Expression* transformVariable(Variable const* var)
    {
        Expression* e = new Variable(var->name());
        return e;
    }
    ~CopySyntaxTree() {};
};

struct FoldConstants : Transformer
{
    Expression* transformNumber(Number const* number)
    {
        Expression* e = new Number(number->value());
        return e;
        // просто возвращаем копию числа
    }
    Expression* transformBinaryOperation(BinaryOperation const* op)
    {
        Expression* l = (op->left())->transform(this); // рекурсивно переходим в левый операнд, чтобы свернуть
        Expression* r = (op->right())->transform(this); // рекурсивно переходим в правый операнд, чтобы свернуть
        int operation = op->operation();

        // Создаем новый объект типа BinaryOperation с новыми указателями
        BinaryOperation* binop = new BinaryOperation(l, operation, r);


        //Проверяем на приводимость указателей к типу Number
        Number* left_num = dynamic_cast<Number*>(l);
        Number* right_num = dynamic_cast<Number*>(r);
        if (left_num && right_num) {
            Expression* result = new Number(op->evaluate());
            
            delete binop;
           
            return result;//вывод уже числа
        }
        else {
            return binop;
        }
    }
    Expression* transformFunctionCall(FunctionCall const* fcall)
    {
        Expression* arg = (fcall->arg())->transform(this);// рекурсивно сворачиваем аргумент
        string const& name_ = fcall->name();

        // Создаем новый объект типа FunctionCall с новым указателем
        FunctionCall* fcall_ = new FunctionCall(name_, arg);

        // Проверяем на приводимость указателя к типу Number
        Number* arg_num = dynamic_cast<Number*>(arg);
        if (arg_num) { // если аргумент — число
            Expression* result = new Number(fcall->evaluate());

            delete fcall_;
            return result;
        }
        else {
            return fcall_;
        }
    }
    Expression* transformVariable(Variable const* var)
    {
        Expression* e = new Variable(var->name());
        return e;  // возвращаем копию переменных
    }
};

//Назначение: посетитель (visitor) позволяет определить новую
//операцию для объектов, не изменяя исходную иерархию классов этих объектов.


int main()
{
    Number* n32 = new Number(32.0);
    Number* n16 = new Number(16.0);
    BinaryOperation* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
    FunctionCall* callSqrt = new FunctionCall("sqrt", minus);
    Variable* var = new Variable("var");
    BinaryOperation* mult = new BinaryOperation(var, BinaryOperation::MUL, callSqrt);
    FunctionCall* callAbs = new FunctionCall("abs", mult);

    CopySyntaxTree CopyTr;
    Expression* Exp = callAbs->transform(&CopyTr);
    cout << Exp -> print() << endl;

    FoldConstants FoldC;
    Expression* Expr = callAbs->transform(&FoldC);
    cout << Expr->print() << endl;

    /*Expression* e1 = new Number(1.234);
    Expression* e2 = new Number(-1.234);
    Expression* e3 = new BinaryOperation(e1, BinaryOperation::DIV, e2);
    cout << e3->evaluate() << endl;*/

    /* Expression* n32 = new Number(32.0);
    Expression* n16 = new Number(16.0);
    Expression* minus = new BinaryOperation(n32, BinaryOperation::MINUS, n16);
    Expression* callSqrt = new FunctionCall("sqrt", minus);
    Expression* n2 = new Number(2.0);
    Expression* mult = new BinaryOperation(n2, BinaryOperation::MUL, callSqrt);
    Expression* callAbs = new FunctionCall("abs", mult);
    cout << callAbs->evaluate() << endl; */
}

