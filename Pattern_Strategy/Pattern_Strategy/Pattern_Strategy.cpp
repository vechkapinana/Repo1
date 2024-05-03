#include <iostream>

using namespace std;

class PrintStrategy {    //класс, выполняющий роль интерфейса
public:
    virtual void operation(int*& arr, const int size) const = 0;   //Для того, чтобы работать не с копией указателя, параметр ссылка на указатель
    virtual ~PrintStrategy() {};
};

/*СТРАТЕГИИ*/
class PrintElementsN : public PrintStrategy {
public:
    PrintElementsN(int n) : n(n) {};
    void operation(int*& arr, const int size) const {
        for (int i = 0; i < size; i++) {
            cout << arr[i] << " ";
            if ((i + 1) % n == 0) {
                cout << endl;
            }
        }
        cout << endl;
    }

    private:
        int n;
};

class PrintElements : public PrintStrategy {
public:
    void operation(int*& arr, const int size) const {
        for (int i = 0; i < size; i++) {
            cout << arr[i] << endl;
        }
    }
};


// Контекст хранит все необходимые алгоритму данные и указатель на объект класса Strategy
class ArrayPrinter {        
    
public:

    ArrayPrinter(PrintStrategy* strategy) : strategy(strategy) {}

    void setStrategy(PrintStrategy* newStrategy) {
        strategy = newStrategy;
    }

    void printArray(int*& arr, const int size) const {
        strategy->operation(arr, size);
    }

private:
    PrintStrategy* strategy;

};

int main()
{
    int size;
    cin >> size;
    int* arr = new int[size];

    for (int i = 0; i < size; i++) {
        cin >> arr[i];
    }
    
    // Создаем объекты стратегий 
    PrintElementsN printN(3);
    PrintElements printOne;

    // Создаем объекты контекста с разными стратегиями 
    ArrayPrinter arrayPrinter1(&printN);
    ArrayPrinter arrayPrinter2(&printOne);

    // Печать массива с разными стратегиями 
    cout << "Printing array 1:" << endl;
    arrayPrinter1.printArray(arr, size);

    cout << "\nPrinting array 2:" << endl;
    arrayPrinter2.printArray(arr, size);

   // PrintStrategy* str1 = new PrintElementsN(3);
   // PrintStrategy* str2 = new PrintElements();


    delete arr;
}

