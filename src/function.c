
Tok t = VAR;
IFSTATE ifstate = CHECK_VAR;
Variable vv;
Variable *vptr = &vv;
COMPARE_IF ci;
COMPARE_IF *cif = &ci;
States state = FIND_CLASS;


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
        /*
        // TODO: FINISH ADDING STRINGS
        else if ((*toks[i] >= 'A' && *toks[i] <= 'Z')||(*toks[i] >= 'a' && *toks[i] <= 'z'))
        {
          printf("string\n");
          exit(0);
        }
        */
        else
        {ERROR_FOUND(15); printf("->%s\n", toks[i]); exit(1);}

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
      printf("%d\n", vptr->val[a]);
      return;
    }

  }
    ERROR_FOUND(6);
    printf("->%s\n", varname);
}


int if_statment(const char *comp)
{
    switch (ifstate)
    {
      case CHECK_VAR:
        if (sscanf(comp, "@%s", cif->comp_var[state_if_count]) == 1)
        {
          for (a=0;a<var_count;a++)
          {
            // FIX CHANGING STATE PROBLEM
            if (strcmp(cif->comp_var[state_if_count], vptr->var_name[a]) == 0)
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
        }
        for (a=0;a<var_count;a++)
        {
            if (cif->comp_val[state_if_count] == vptr->val[a])
            {
              //printf("Same NUM\n");
              return 2;
              break;
            }

        }
        //printf("NOT SAME\n");
        return 1;
      break;

    }
}
