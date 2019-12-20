#ifndef TYPEH
#define TYPEH

enum Types {
	TYPE_ERROR = 0, INT, NEGINT, 
	FLOAT, NEGFLOAT, STRING
};

enum Types typeDetect(struct Scan *s)
{
	char type[20] = "\0";
	char* idx = s->lexeme;
	bool dbl = false;
	bool neg = false;

	while (*idx)
	{
		if (*idx == '\'')
		{
			return STRING;
		}
		if (*idx == '-')
		{
			neg = true;
			idx++;
		}
		if (*idx >= '0' && *idx <= '9')
		{
			while (*idx)
			{
				if (*idx == '.') 
				{
					dbl = true;
					append(type, '.');
				}
				if (*idx >= '0' && *idx <= '9')
					append(type, *idx);
				idx++;
			}
			if (dbl == true)
			{
				if (neg == true)
					return NEGFLOAT;
				return FLOAT;
			}
			if (neg == true)
				return NEGINT;
			return INT;
		}
		idx++;
	}
	return TYPE_ERROR;
}

const int intType(char *str)
{
	return atoi(str);
}

const double dblType(char *str)
{
	return atof(str);
}

#endif
