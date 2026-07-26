%{
#include <stdio.h>
#include <string.h>

FILE *out;
char link_url[500] = "";
int in_anchor = 0;

void extractURL(char *str)
{
    char *start = strstr(str, "href=\"");

    if (start == NULL)
    {
        fprintf(out, "Error: Anchor tag found without href attribute.\n\n");
        in_anchor = 0;
        return;
    }

    start += 6;
    int i = 0;
    while (start[i] != '"' && start[i] != '\0' && i < 499)
    {
        link_url[i] = start[i];
        i++;
    }
    link_url[i] = '\0';

    if (strlen(link_url) == 0)
    {
        fprintf(out, "Error: Empty href attribute.\n\n");
        in_anchor = 0;
    }
    else
    {
        in_anchor = 1;
    }
}
%}

%option noyywrap

%%

"<a"[^>]*">" {
    extractURL(yytext);
}

"</a>" {
    in_anchor = 0;
}

[^<\n]+ {
    if (in_anchor)
    {
        fprintf(out, "Link Text: %s\n", yytext);
        fprintf(out, "URL: %s\n", link_url);

        if (strcmp(link_url, "#") == 0)
            fprintf(out, "Note: Hyperlink points to the current page.\n");

        fprintf(out, "\n");

        link_url[0] = '\0';
    }
}

.      ;
\n     ;

%%

int main()
{
    FILE *in = fopen("web_input.html", "r");
    if (in == NULL) { printf("Cannot open HTML file.\n"); return 1; }

    out = fopen("web_output.txt", "w");
    if (out == NULL) { printf("Cannot open output file.\n"); fclose(in); return 1; }

    yyin = in;
    yylex();

    printf("Hyperlinks extracted successfully.\n");

    fclose(in);
    fclose(out);
    return 0;
}