%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

FILE *out;
char link_url[500];
int in_anchor = 0;

/* Function to extract URL from href attribute */
void extractURL(char *str)
{
    char *start = strstr(str, "href=\"");
    if(start != NULL)
    {
        start += 6; /* Skip 'href="' */
        int i = 0;
        while(start[i] != '"' && start[i] != '\0')
        {
            link_url[i] = start[i];
            i++;
        }
        link_url[i] = '\0';
    }
}
%}

%%

"<a"[^>]*"href="[^>]*">"  {
    in_anchor = 1;
    extractURL(yytext);
}

"</a>"  {
    in_anchor = 0;
}

[^<\n]+  {
    if(in_anchor && link_url[0] != '\0')
    {
        fprintf(out, "Link Text: %s\n", yytext);
        fprintf(out, "URL: %s\n\n", link_url);
        link_url[0] = '\0';
    }
}

.|\n  ;

%%

int yywrap()
{
    return 1;
}

int main()
{
    FILE *in;
    
    in = fopen("webpage.html", "r");
    if(in == NULL)
    {
        printf("Cannot open HTML file.\n");
        return 1;
    }
    
    out = fopen("links.txt", "w");
    if(out == NULL)
    {
        printf("Cannot open output file.\n");
        return 1;
    }
    
    yyin = in;
    yylex();
    
    printf("Hyperlinks extracted successfully.\n");
    
    fclose(in);
    fclose(out);
    
    return 0;
}
