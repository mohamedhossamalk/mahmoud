#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <cctype>
using namespace std;

// افتراض أن كلاس Stack موجود بالفعل
// ويحتوي على الدوال الأساسية:
// push(), pop(), peek(), isEmpty()

// دالة للتحقق إذا كان الرمز عملية حسابية
bool isOperator(char c) {
    return (c == '+' || c == '-' || c == '*' || c == '/' || c == '^');
}

// دالة لتحديد أولوية العمليات الحسابية
int precedence(char op) {
    if (op == '^') return 3;
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return -1;
}

// تحويل معادلة من infix إلى prefix
string infixToPrefix(string infix) {
    // عكس المعادلة للمساعدة في التحويل
    reverse(infix.begin(), infix.end());
    
    // تغيير الأقواس المعكوسة
    for (int i = 0; i < infix.length(); i++) {
        if (infix[i] == '(') 
            infix[i] = ')';
        else if (infix[i] == ')') 
            infix[i] = '(';
    }
    
    Stack operatorStack;
    string prefix = "";
    
    for (int i = 0; i < infix.length(); i++) {
        // إذا كان الرمز الحالي رقم أو حرف، نضيفه إلى النتيجة
        if (isalnum(infix[i])) {
            prefix += infix[i];
        }
        // إذا كان قوساً مفتوحاً، نضيفه للـ Stack
        else if (infix[i] == '(') {
            operatorStack.push(infix[i]);
        }
        // إذا كان قوساً مغلقاً، نخرج من الـ Stack حتى نجد القوس المفتوح
        else if (infix[i] == ')') {
            while (!operatorStack.isEmpty() && operatorStack.peek() != '(') {
                prefix += operatorStack.pop();
            }
            
            // إزالة القوس المفتوح من الـ Stack
            if (!operatorStack.isEmpty()) {
                operatorStack.pop();
            }
        }
        // إذا كان عملية حسابية
        else if (isOperator(infix[i])) {
            while (!operatorStack.isEmpty() && precedence(infix[i]) < precedence(operatorStack.peek())) {
                prefix += operatorStack.pop();
            }
            operatorStack.push(infix[i]);
        }
    }
    
    // إضافة باقي العناصر من الـ Stack إلى النتيجة
    while (!operatorStack.isEmpty()) {
        if (operatorStack.peek() != '(') {
            prefix += operatorStack.pop();
        } else {
            operatorStack.pop();
        }
    }
    
    // عكس النتيجة للحصول على الـ prefix الصحيح
    reverse(prefix.begin(), prefix.end());
    
    return prefix;
}

// حساب قيمة معادلة prefix
double evaluatePrefix(string prefix) {
    Stack operandStack;
    
    // قراءة المعادلة من اليمين إلى اليسار
    for (int i = prefix.length() - 1; i >= 0; i--) {
        // إذا كان الرمز الحالي رقم
        if (isdigit(prefix[i])) {
            operandStack.push(prefix[i] - '0');  // تحويل من char إلى int
        }
        // إذا كان الرمز الحالي عملية حسابية
        else if (isOperator(prefix[i])) {
            double operand1 = operandStack.pop();
            double operand2 = operandStack.pop();
            
            switch (prefix[i]) {
                case '+': operandStack.push(operand1 + operand2); break;
                case '-': operandStack.push(operand1 - operand2); break;
                case '*': operandStack.push(operand1 * operand2); break;
                case '/': operandStack.push(operand1 / operand2); break;
                case '^': operandStack.push(pow(operand1, operand2)); break;
            }
        }
    }
    
    // النتيجة النهائية ستكون في أعلى الـ Stack
    return operandStack.pop();
}

// دالة موحدة تقوم بتحويل المعادلة وحسابها
double processEquation(string infix) {
    string prefix = infixToPrefix(infix);
    cout << "Infix: " << infix << endl;
    cout << "Prefix: " << prefix << endl;
    double result = evaluatePrefix(prefix);
    cout << "Result: " << result << endl;
    return result;
}

// دالة الاختبار
void testEquations() {
    processEquation("3+4*2");          // (3+(4*2)) = 11
    processEquation("(5+3)*2");        // ((5+3)*2) = 16
    processEquation("5+3*2^2-6/2");    // (5+(3*(2^2))-(6/2)) = 14
}

// دالة main - يمكن استبدالها بما يناسب البرنامج الخاص بك
int main() {
    testEquations();
    
    // للتفاعل مع المستخدم
    string equation;
    cout << "Enter an infix equation: ";
    getline(cin, equation);
    processEquation(equation);
    
    return 0;
}