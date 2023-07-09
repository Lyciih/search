#include "flow.h"

int analysis(int capture_number, char * pattern, char * data, char * buffer, int buffer_size)
{
	int regex_state;
	regex_t find;
	regmatch_t match[capture_number + 1];

	regex_state = regcomp(&find, pattern, REG_EXTENDED);
	if(regex_state != 0)
	{
		regerror(regex_state, &find, buffer, buffer_size);
		printf("%s\n", buffer);
		return -1;
	}
	else
	{

		regex_state = regexec(&find, data, capture_number + 1, match, 0);
		if(regex_state == REG_NOMATCH)
		{
			regerror(regex_state, &find, buffer, buffer_size);
			printf("%s\n", buffer);
			return -2;
		}
		else if(regex_state == 0)
		{
			int j = 0;
			for(int i = match[capture_number].rm_so; i < match[capture_number].rm_eo; i++)
			{
				//printf("%c", data[i]);
				buffer[j] = data[i];
				j++;
				if(j == buffer_size - 1)
				{
					buffer[j] = '\0';
					printf("buffer size too small: %d\n", buffer_size);
					return -3;
				}
			}
			buffer[j] = '\0';
		}
		regfree(&find);
		return 0;
	}
}
