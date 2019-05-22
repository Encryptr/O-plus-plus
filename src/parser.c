#ifndef PARSE
#define PARSE

Tok t = VAR;
IFSTATE ifstate = CHECK_VAR;
Variable vv;
Variable *vptr = &vv;
COMPARE_IF ci;
COMPARE_IF *cif = &ci;
States state = FIND_ALL;
Change_Var cv;
Change_Var *pcv = &cv;
Order_Change order = IDENTIFY;
Global_Var gva;
Global_Var *pva = &gva;
Func_Var fv;
Func_Var *pfv = &fv;

static void append(char *original, char *add)
{
 while(*original)
    original++;
   
 while(*add)
 {
    *original = *add;
    add++;
    original++;
 }
 *original = '\0';
}

void lex_class(char toks[100][100])
{
  if (strcmp(*toks, "") == 0)
  {
    ERROR_FOUND(1);
    exit(1);
  }

  for (i=0;i<idx;i++)
  {
    switch (t)
    {
      case VAR:
        if (sscanf(toks[i], "@%s", vptr->var_name[var_count]) == 1)
        {
          for (a=0;a<var_count;a++)
          {
            if (strcmp(vptr->var_name[var_count], vptr->var_name[a]) == 0)
            {
              ERROR_FOUND(13); printf("-> %s\n", vptr->var_name[var_count]); exit(1);
            }
          }
          t = EQ;
        }
        else
        {
          ERROR_FOUND(2);
          exit(1);
        }
      break;

      case EQ:
        if (strcmp(toks[i], ":") == 0)
        {
          t = TYPE;
        }
        else {ERROR_FOUND(3); exit(1);}

      break;

      case TYPE:
        if (*toks[i] >= '0' && *toks[i] <= '9')
        {
          int num = atoi(toks[i]);
          vptr->val[var_count] = num;
          var_count++;
        }
        else if (sscanf(toks[i], " ' %[^'\n] ' ", vptr->string[var_count]) == 1)
        {
          is_string++;
          var_count++;
        }
        else {ERROR_FOUND(15); printf("->%s\n", toks[i]); exit(1);}

        t = VAR;

      break;

    }
  }
  if (i != var_count * 3)
  {
    ERROR_FOUND(12); exit(1);
  }
}

void print_var(const char *varname)
{
  for (a=0;a<var_count;a++)
  {
    if (strcmp(varname, vptr->var_name[a]) == 0)
    {
      if (is_string >= 1)
      {
        printf("%s\n", vptr->string[a]);
        is_string--;
        return;
      }
      printf("%d\n", vptr->val[a]);
      return;
    }

  }
    ERROR_FOUND(6);
    printf("->%s\n", varname);
}

// FIX PRINT IN FUNCTIONS
void create_func(char* c)
{
  if (strcmp(c, "endfun") == 0)
  {
    pfv->fvar_amount++;
    done_func = 1;
    return;
  }
  append(func_tokens[pfv->fvar_amount], c);
  append(func_tokens[pfv->fvar_amount], "\n");
}

int if_statment(const char *comp)
{
    int var_idx;
    switch (ifstate)
    {
      case CHECK_VAR:
      // TODO: ADD IF STATMENT FOR STRINGS
        if (sscanf(comp, "@%s", cif->comp_var[state_if_count]) == 1)
        {
          for (var_idx=0;var_idx<var_count;var_idx++)
          {
            // FIX CHANGING STATE PROBLEM
            if (strcmp(cif->comp_var[state_if_count], vptr->var_name[var_idx]) == 0)
            {
              //printf("ITS THE SAME\n");
              ifstate = EQEQ;
              break;
            }
          }
          if (ifstate != EQEQ) {
            ERROR_FOUND(8); printf("-> %s\n", comp); exit(1);
          }
        }
        else {ERROR_FOUND(9); printf("-> %s\n", comp); exit(1);}
      break;

      case EQEQ:
        if (strcmp(comp, "::") == 0)
        {
          // printf("READY TO CHECK NUM\n");
          ifstate = NUM_CHECK;
        }
        else {ERROR_FOUND(3); printf("-> %s\n", comp); exit(1);}
      break;

      case NUM_CHECK:
        if (*comp >= '0' && *comp <= '9')
        {
          int value = atoi(comp);
          cif->comp_val[state_if_count] = value;
          ifstate = CHECK_VAR;
        }
        else {ERROR_FOUND(19); printf("->%s\n", comp); exit(1);}
        // ADD EXIT
          if (cif->comp_val[state_if_count] == vptr->val[var_idx])
          {
            //printf("Same NUM\n");
            return 2;
            break;
          }

        //printf("NOT SAME\n");
        return 1;
      break;

    }

    return 0;
}

void change_variable(const char *curr)
{
  switch (order)
  {
    case IDENTIFY:
      if (strcmp(curr, operators[0]) == 0)
      {pcv->op_result = 1;}
      else if (strcmp(curr, operators[1]) == 0)
      {pcv->op_result = 2;}
      else if (strcmp(curr, operators[2]) == 0)
      {pcv->op_result = 3;}
      else if (strcmp(curr, operators[3]) == 0)
      {pcv->op_result = 4;}
      else {ERROR_FOUND(18); printf("->%s\n", curr); exit(1);}
      order = EXECUTE;
    break;

    case EXECUTE:
    // TODO: INPLEMENT FLOATS
    // if (sscanf(curr, " %f ", pcv->change_by_num[change_count]) == 1)
    // {
    //   pcv->change_by_num[change_count] = strtof(curr, NULL);
    //   printf("%d\n",vptr->val[pcv->which_match]);
    //   //pcv->change_by_num[change_count] + vptr->val[pcv->which_match];
    // }
    // TODO: ADD MINUS STATES MULTIPLY AND MORE
    // TODO: ADD VAR + VAR
    if (*curr >= '0' && *curr <= '9')
    {
      pcv->change_by_num[change_count] = atoi(curr);
      vptr->val[pcv->which_match] += pcv->change_by_num[change_count];
      // USE CONT TO TELL IF KEEP GOING
      pcv->cont = 1;
      change_count++;
      order = IDENTIFY;
      return;
    }
    else {ERROR_FOUND(-1); printf("->%s\n", curr);exit(1);}
    break;
  }
  return;
}

// REVIEW BUT WORKING
void call_func(const char* c)
{
  if (pfv->done_call == 0)
  {
    for (a=0;a<pfv->fvar_amount;a++)
    {
      if (strcmp(c, pfv->fvar[a]) == 0)
      {
        pfv->done_call = 1;
        goto sucsess;
        break;
      }
    }
    printf("No FUNCTION with that name -> %s\n",c); exit(1);
  }

  sucsess: 
  if (pfv->done_call == 1)
  {
    main_lex(func_tokens[a]);
    pfv->done_call = 0;
  }
}

#endif