/* UZ SE NEPOUZIVA */
#include "InfixPostfixSolve.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "List.h"

/* Interni funkce pro dealokaci parametru */
inline void free_param_vars(float *pv_a, float *pv_b) {
  free(pv_a);
  free(pv_b);
}
 
/** Vrati A + B jako dynamicky alokovanou promennou
 *  @param a Ukazatel na vstupni promennou A
 *  @param b Ukazatel na vstupni promennou B
 *  @return Ukazatel na vysledek operace
 */
float* fsum(float *a, float *b) {
  float *res = (float *) malloc (sizeof(float));
  *res = *a + *b;
  free_param_vars(a, b);
  return res;
}

/** Vrati A - B jako dynamicky alokovanou promennou
 *  @param a Ukazatel na vstupni promennou A
 *  @param b Ukazatel na vstupni promennou B
 *  @return Ukazatel na vysledek operace
 */
float* fsub(float *a, float *b) {
  float *res = (float *) malloc (sizeof(float));
  *res = *a - *b;
  free_param_vars(a, b);
  return res;
}

/** Vrati A * B jako dynamicky alokovanou promennou
 *  @param a Ukazatel na vstupni promennou A
 *  @param b Ukazatel na vstupni promennou B
 *  @return Ukazatel na vysledek operace
 */
float* fmult(float *a, float *b) {
  float *res = (float *) malloc (sizeof(float));
  *res = *a * *b;
  free_param_vars(a, b);
  return res;
}

/** Vrati A / B jako dynamicky alokovanou promennou
 *  @param a Ukazatel na vstupni promennou A
 *  @param b Ukazatel na vstupni promennou B
 *  @return Ukazatel na vysledek operace
 */
float* fdiv(float *a, float *b) {
  float *res = (float *) malloc (sizeof(float));
  *res = *a / *b;
  free_param_vars(a, b);
  return res;
}

/** Vypocte postfix vyraz 
 *  @param expr Vyraz k vyhodnoceni
 *  @param result Vysledek vyrazu
 *  @return True - vyhodnoceni probehlo v poradku, False - chyba pri vyhodnocovani vyrazu
 */
bool solve_postfix(const char *expr, float *result) {
  struct list *stack = newList();
  char buff[10] = "";
  int index = 0;
  while (1) {
    if ((*expr >= '0' && *expr <= '9') || *expr == '.') { // ciselna hodnota
      buff[index] = *expr;
      ++index;

    } else if (*expr == '/' || *expr == '*' || *expr == '+' || *expr == '-') { // operator
      if (get_count(stack) > 1) { // kontrola, zda mam dost parametru k provedeni funkce
        float* par[2];
        for (int i = 1; i >= 0; --i) {
          par[i] = (float*)remove_last(stack);
        }
        switch (*expr) {
          case '/':
            insert(stack, (void*)fdiv(par[0], par[1]));
            break;
          case '*':
            insert(stack, (void*)fmult(par[0], par[1]));
            break;
          case '+':
            insert(stack, (void*)fsum(par[0], par[1]));
            break;
          case '-':
            insert(stack, (void*)fsub(par[0], par[1]));
            break;
        }
        
      } else {
        list_dealloc(stack);
        return false;
      }

    } else if (*expr == ' ' || *expr == '\0') { // mezera nebo konec vyrazu
      if (index) { /* pokud byla ukladana hodnota do bufferu */
        buff[index] = '\0'; // uzavri string za touho hodnotou
        // preved tuto hodnotu na float
        float *value = (float *) malloc (sizeof(float));
        *value = atof(buff);
        // vloz do zasobniku
        insert(stack, (void*)value);
        // a vymaz buffer
        strcpy(buff, "");
        index = 0;
      }

      // pokud je konec vyrazu
      if (*expr == '\0') {
        if (get_count(stack) == 1) {
          float *res = (float*)remove_last(stack);
          *result = *res;
          free(res);
          list_dealloc(stack);
          return true;
        } else {
          list_dealloc(stack);
          return false;
        }
      }
    }
    ++expr; // prejit na dalsi znak
  }
}

/* Zjisti, zda je dany operator platny */
bool is_operator(const char o) {
  if (o == '*' || o == '/' || o == '+' || o == '-') {
    return true;
  }
  return false;
}

/* urci prioritu operatoru */
int priority(char p) {
  if (p == '(')
    return 0;
  if (p == '+' || p == '-')
    return 1;
  if (p == '*' || p == '/' || p == '%')
    return 2;
  
  return 3;
}


/** Prevede infix vzorec na postfix vzorec
 *  @param infix Vstupni infix vzorec
 *  @param postfix Vystupni postfix vzorec
 *  @return True - převod úspěšný, False - chyba v převodu
 */
bool infix_to_postfix(const char *infix, char *postfix) {
  struct list *s = newList(); // novy list (=zasobnik)
  char *c;
  char x; 
  char token; // promenna do ktere si ukladam jednotlive tokeny (char znaky)
  int i;
  int j = 0;

  for (i=0; infix[i] != '\0'; ++i) { // prochazim cely infix vzorecek
    token=infix[i]; // ulozim si znak pro aktualni cyklus jako token
    
    if ( (token >= '0' && token <= '9') || token == '.' || token == 'V' ) { // pokud je token ciselna hodnota
      postfix[j++] = token; // pridej k postfixu tento token
      
    } else if (token == '(') {
        postfix[j++] = ' ';
        c = (char*)malloc(sizeof(char));
        *c = '(';
        insert(s, c);
      
    } else if (token == ')') {
        postfix[j++] = ' ';

        
        c = (char *) get_last(s); 
        if (*c == NULL) return false;/* Pokud je zasobnik prazdny, tak se jedna o prazdnou zavorku - CHYBA */
        
        while((c = (char *)remove_last(s)) != NULL && *c !='(' ) {
          postfix[j++] = *c;
          free(c);
          
        }
        if (*c != '(') {
          free(c);
          return false; /* pokud nebyla nalezena v zasobniku oteviraci zavorka - CHYBA */
        } else {
          free(c);
        }
        
      }else if (token == ' ') {
        postfix[j++] = ' ';
          
      } else {
        postfix[j++] = ' ';
        while( (priority(token) <= priority(*(char *)get_last(s))) && get_count(s) ) {
          c = (char *)remove_last(s);
          postfix[j++] = *c;
          free(c);
        }
        c = (char *) malloc (sizeof(char));
        *c = token;
        insert(s, c);
      }
    }
    postfix[j++] = ' ';
    while(get_count(s)) {
      c = (char *)remove_last(s);
      if (*c == '(') {
        free(c);
        return false; // neuzavrena zavorka a na vstupu uz nic neni - CHYBA
      }
      postfix[j++] = *c;
      free(c);
    }
    postfix[j] = '\0';
    list_dealloc(s);
    return true;
    
}
