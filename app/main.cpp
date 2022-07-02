#include <iostream>
#include <evaluator/Context.h>
using namespace eval;

void printList(const ListType &l)
{
    std::cout << '[';
    if (!l.empty())
        std::cout << l[0];
    for (size_t i = 1; i < l.size(); ++i)
        std::cout << ", " << l[i];
    std::cout << "]\n";
}

void printLambdaSig(const LambdaType &l)
{
    std::cout << "@(";
    if (!l.params.empty())
        std::cout << l.params[0];
    for (size_t i = 1; i < l.params.size(); ++i)
        std::cout << ", " << l.params[i];
    std::cout << "){...}\n";
}

int main()
{
    Context context;
    context.init();
    while (true)
    {
        std::cout << "eval> ";
        std::string input;
        std::getline(std::cin, input);

        if (input.empty())
            continue;
        if (input[0] == '!')
        {
            auto cmd = input.substr(1);
            if (cmd == "exit")
            {
                return 0;
            }
            else if (cmd == "list")
            {
                for (auto &v : context.varMap())
                    std::cout << v.first << ", ";
                std::cout << '\n';
            }
            else if (cmd == "init")
            {
                context.init();
            }
            else
            {
                std::cout << "unknown command\n";
            }
            continue;
        }

        DataType ret;
        try
        {
            ret = context.exec(input);
#ifndef NDEBUG
            std::cout << context.AST()->toJson().toStringFormatted() << '\n';
#endif
        }
        catch (const EvalExcept &e)
        {
            std::cerr << e.what() << '\n';
        }
        switch (ret.index())
        {
        case 1:
            std::cout << " = " << std::get<1>(ret) << '\n';
            break;
        case 2:
            std::cout << " = ";
            printList(std::get<2>(ret));
            break;
        case 3:
            std::cout << " = ";
            printLambdaSig(std::get<3>(ret));
            break;
        }
    }
}