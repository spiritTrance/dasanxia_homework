bool frontend::DFA::next(char input, Token &buf)
{
#ifdef DEBUG_DFA
#include <iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = " << cur_str << "\t"<<std::endl;
#endif
    bool ret;
    switch (cur_state)
    {
    case State::Empty:
        if (isoperator(input))
        {
            cur_state = State::op;
            cur_str += input;
            ret = false;
        }
        else if (isnumber(input))
        {
            cur_state = State::IntLiteral;
            cur_str += input;
            ret = false;
        }
        else if (isletter(input) || input == '_')
        {
            cur_state = State::Ident;
            cur_str += input;
            ret = false;
        }
        else
        {
            ret = false;
        }
        break;

    case State::Ident:
        if (isletter(input) || isnumber(input) || input == '_')
        {
            cur_str += input;
            ret = false;
        }
        else
        {
            if (keywords.find(cur_str) != keywords.end())
            {
                buf.type = get_on_type(cur_str);
                buf.value = cur_str;
            }
            else
            {
                buf.type = TokenType::IDENFR;
                buf.value = cur_str;
            }
            cur_str = "";
            cur_state = State::Empty;
            ret = true;
        }
        break;

    case State::IntLiteral:
        if (isnumber(input))
        {
            cur_str += input;
            ret = false;
        }
        else if (input == '.')
        {
            cur_state = State::FloatLiteral;
            cur_str += input;
            ret = false;
        }
        else if(isoperator(input))
        {
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;
            cur_str = input;
            cur_state = State::op;
            ret = true;
        }
        else
        {
            buf.type = TokenType::INTLTR;
            buf.value = cur_str;
            cur_str = "";
            cur_state = State::Empty;
            ret = true;
        }
        break;

    case State::FloatLiteral:
        if (isnumber(input))
        {
            cur_str += input;
            ret = false;
        }
        else
        {
            buf.type = TokenType::FLOATLTR;
            buf.value = cur_str;
            cur_str = "";
            cur_state = State::Empty;
            ret = true;
        }
        break;

    case State::op:
        if (isoperator(input))
        {
            if(input=='='||input=='&'||input=='|')
            {
                cur_str += input;
                ret = false;
            }
            else
            {
                buf.type = get_on_type(cur_str);
                buf.value = cur_str;
                cur_str = input;
                cur_state = State::op;
                ret = true;
            }
        }
        else
        {
            buf.type = get_on_type(cur_str);
            buf.value = cur_str;
            cur_str = "";
            cur_state = State::Empty;
            ret = true;
        }
        break;

    default:
        break;
    }
#ifdef DEBUG_DFA
    std::cout << "next state is [" << toString(cur_state) << "], next str = " << cur_str << "\t, ret = " << ret << std::endl;
#endif
    return ret;
}