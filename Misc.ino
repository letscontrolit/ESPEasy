 /********************************************************************************************\
 * Een float en een unsigned long zijn beide 4bytes groot. Deze zijn niet te casten naar 
 * elkaar. Onderstaande twee funkties converteren de unsigned long
 * en de float.
 \*********************************************************************************************/
unsigned long float2ul(float f)
  {
  unsigned long ul;
  memcpy(&ul, &f,4);
  return ul;
  }

float ul2float(unsigned long ul)
  {
  float f;
  memcpy(&f, &ul,4);
  return f;
  }
  
void addLog(byte loglevel, char *line)
{
  if (loglevel <= Settings.SerialLogLevel)
    Serial.println(line);
    
  if (loglevel <= Settings.SyslogLevel)
    syslog(line);

  if (loglevel <= Settings.WebLogLevel)
    {
      logcount++;
      if (logcount > 9)
        logcount = 0;
      Logging[logcount].timeStamp = millis();
      strncpy(Logging[logcount].Message, line, 80);
      Logging[logcount].Message[79] = 0;
    }
}


void delayedReboot(int rebootDelay)
  {
    while (rebootDelay !=0 )
    {
      Serial.print(F("Delayed Reset "));
      Serial.println(rebootDelay);
      rebootDelay--;
      delay(1000);
    }
    ESP.reset();
  }

//################### Calculate #################################
#define CALCULATE_OK                            0
#define CALCULATE_ERROR_STACK_OVERFLOW          1
#define CALCULATE_ERROR_BAD_OPERATOR            2
#define CALCULATE_ERROR_PARENTHESES_MISMATCHED  3
#define CALCULATE_ERROR_UNKNOWN_TOKEN           4
#define STACK_SIZE 10 // was 50
#define TOKEN_MAX 20

float globalstack[STACK_SIZE];
float *sp = globalstack-1;
float *sp_max = &globalstack[STACK_SIZE-1];

#define is_operator(c)  (c == '+' || c == '-' || c == '*' || c == '/' )

int push(float value)
{
  if(sp != sp_max) // Full
  {
    *(++sp) = value;
    return 0;
  }
  else 
    return CALCULATE_ERROR_STACK_OVERFLOW;
}

float pop()
{
  if(sp != (globalstack-1)) // empty
    return *(sp--);
}

float apply_operator(char op, float first, float second)
  {
  switch(op)
  {
  case '+': 
    return first + second;
  case '-': 
    return first - second;
  case '*': 
    return first * second;
  case '/': 
    return first / second;
    return 0;
  }  
}

char *next_token(char *linep)
{
  while(isspace(*(linep++)));
  while(*linep && !isspace(*(linep++)));
  return linep;
}

int RPNCalculate(char* token)
{
  if(token[0]==0)
    return 0; // geen moeite doen voor een lege string

  if(is_operator(token[0]))
  {
    float second = pop();
    float first = pop();

    if(push(apply_operator(token[0], first, second)))
      return CALCULATE_ERROR_STACK_OVERFLOW;
  }
  else // Als er nog een is, dan deze ophalen
  if(push(atof(token))) // is het een waarde, dan op de stack plaatsen
      return CALCULATE_ERROR_STACK_OVERFLOW;

  return 0;
}

// operators
// precedence   operators         associativity
// 3            !                 right to left
// 2            * / %             left to right
// 1            + - ^             left to right
int op_preced(const char c)
{
  switch(c)    
  {
  case '*':  
  case '/': 
    return 2;
  case '+': 
  case '-': 
    return 1;
  }
  return 0;
}

bool op_left_assoc(const char c)
{
  switch(c)
  { 
  case '*': 
  case '/': 
  case '+': 
  case '-': 
    return true;     // left to right
    //case '!': return false;    // right to left
  }
  return false;
}

unsigned int op_arg_count(const char c)
{
  switch(c)  
  {
  case '*': 
  case '/': 
  case '+': 
  case '-': 
    return 2;
    //case '!': return 1;
  }
  return 0;
}


int Calculate(const char *input, float* result)
{
  const char *strpos = input, *strend = input + strlen(input);
  char token[25];
  char c, *TokenPos = token;
  char stack[32];       // operator stack
  unsigned int sl = 0;  // stack length
  char     sc;          // used for record stack element
  int error=0;

  //*sp=0; // bug, it stops calculating after 50 times
  sp = globalstack-1;
  
  while(strpos < strend)   
  {
    // read one token from the input stream
    c = *strpos;
    if(c != ' ')
    {
      // If the token is a number (identifier), then add it to the token queue.
      if((c >= '0' && c <= '9') || c=='.')
      {
        *TokenPos = c; 
        ++TokenPos;
      }

      // If the token is an operator, op1, then:
      else if(is_operator(c))
      {
        *(TokenPos)=0;
        error=RPNCalculate(token);
        TokenPos=token;
        if(error)return error;
        while(sl > 0)
        {
          sc = stack[sl - 1];
          // While there is an operator token, op2, at the top of the stack
          // op1 is left-associative and its precedence is less than or equal to that of op2,
          // or op1 has precedence less than that of op2,
          // The differing operator priority decides pop / push
          // If 2 operators have equal priority then associativity decides.
          if(is_operator(sc) && ((op_left_assoc(c) && (op_preced(c) <= op_preced(sc))) || (op_preced(c) < op_preced(sc))))
          {
            // Pop op2 off the stack, onto the token queue;
            *TokenPos = sc; 
            ++TokenPos;
            *(TokenPos)=0;
            error=RPNCalculate(token);
            TokenPos=token; 
            if(error)return error;
            sl--;
          }
          else
            break;
        }
        // push op1 onto the stack.
        stack[sl] = c;
        ++sl;
      }
      // If the token is a left parenthesis, then push it onto the stack.
      else if(c == '(')
      {
        stack[sl] = c;
        ++sl;
      }
      // If the token is a right parenthesis:
      else if(c == ')')
      {
        bool pe = false;
        // Until the token at the top of the stack is a left parenthesis,
        // pop operators off the stack onto the token queue
        while(sl > 0)
        {
          *(TokenPos)=0;
          error=RPNCalculate(token);
          TokenPos=token; 
          if(error)return error;
          sc = stack[sl - 1];
          if(sc == '(')
          {
            pe = true;
            break;
          }
          else  
          {
            *TokenPos = sc; 
            ++TokenPos;
            sl--;
          }
        }
        // If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
        if(!pe)  
          return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

        // Pop the left parenthesis from the stack, but not onto the token queue.
        sl--;

        // If the token at the top of the stack is a function token, pop it onto the token queue.
        if(sl > 0)
          sc = stack[sl - 1];

      }
      else
        return CALCULATE_ERROR_UNKNOWN_TOKEN;
    }
    ++strpos;
  }
  // When there are no more tokens to read:
  // While there are still operator tokens in the stack:
  while(sl > 0)
  {
    sc = stack[sl - 1];
    if(sc == '(' || sc == ')')
      return CALCULATE_ERROR_PARENTHESES_MISMATCHED;

    *(TokenPos)=0;
    error=RPNCalculate(token);
    TokenPos=token; 
    if(error)return error;
    *TokenPos = sc; 
    ++TokenPos;
    --sl;
  }

  *(TokenPos)=0;
  error=RPNCalculate(token);
  TokenPos=token; 
  if(error)
  {
    *result=0;
    return error;
  }  
  *result=*sp;
  return CALCULATE_OK;
}
//################### Einde Calculate #################################

