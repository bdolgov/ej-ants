#include "tournament.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#define len(x) ((int)(x).size())

const int MAXL = 30;

using namespace std;

const char *cells[2] = {"divWinCell", "divLoseCell"};

struct Renderer : IRenderer
{
    void render(const vector<vector<Group*> >& groups);
};

IRenderer* IRenderer::get()
{
    return new Renderer;
}

void make_group_table(char *s, int round_id, int group_id, const Group* group) {
  char t[MAXL];
  sprintf(t, "./tables/%s", s);
  FILE *f = fopen(t, "wt");
  assert(f);
  fprintf(f, "<html>\n");
  
  fprintf(f, "<head>\n");
  fprintf(f, "\t<title>Результаты</title>\n");
  fprintf(f, "\t<meta charset='utf-8'/>\n");
  fprintf(f, "\t<link rel=\"stylesheet\" type=\"text/css\" href=\"../tables.css\">\n");
  fprintf(f, "</head>\n");
  
  fprintf(f, "<body>\n");  
  fprintf(f, "\t<div class=\"divTable\">\n");
  
  fprintf(f, "\t\t<div class=\"divRow\">\n");
  fprintf(f, "\t\t\t<div class=\"divHeadCell\" align=\"center\"> Раунд %d. Группа %d. </div>\n", round_id, group_id);
  fprintf(f, "\t\t</div>\n");
  
  fprintf(f, "\t\t<div class=\"divRow\">\n");
  fprintf(f, "\t\t\t<div class=\"divCell\" align=\"center\"> Бой \\ Команда </div>\n");
  for (int i = 0; i < len(group->participants); i++) {
    fprintf(f, "\t\t\t<div class=\"%s\">%s</div>\n", cells[i / 2], group->participants[i].participant->name.c_str());    
  }
  fprintf(f, "\t\t</div>\n");
  
  for (int i = 0; i < len(group->plays); i++) {
    fprintf(f, "\t\t<div class=\"divRow\">\n");
    fprintf(f, "\t\t\t<div class=\"divCell\"> <a href=\"http://informatics.mccme.ru/ants/visualizer.html?%d\" target=\"_blank\"> #%d </a> </div>\n", 
            group->plays[i].id, i + 1);
    for (int j = 0; j < len(group->participants); j++) {
      int score = -1;
      for (int k = 0; k < len(group->plays[i].players); k++) {
        if (group->participants[j].participant == group->plays[i].players[k].participant) {
          score = group->plays[i].players[k].score;
          break;
        }          
      }  
      fprintf(f, "\t\t\t<div class=\"%s\"> %d </div>\n", cells[j / 2], score);
    }      
    fprintf(f, "\t\t</div>\n");
  }
  
  fprintf(f, "\t\t<div class=\"divRow\">\n");
  fprintf(f, "\t\t\t<div class=\"divCell\"> Сумма </div>\n");
  for (int i = 0; i < len(group->participants); i++) {
    fprintf(f, "\t\t\t<div class=\"%s\"> %d </div>\n", cells[i / 2], group->participants[i].score);
  }
  fprintf(f, "\t\t</div>\n");
  
  
  fprintf(f, "\t</div>\n");
  fprintf(f, "</body>\n");
  fprintf(f, "</html>\n");
  fclose(f);
}

FILE *b;

void make_round(int x, const vector<vector<Group*> >& groups, const vector<vector <string> > &gtables) {
  fprintf(b, "\t<div class=\"divWrap\">\n");
  /*if (x != len(groups) - 1) {
    fprintf(b, "\t\t<div class=\"divHalfFiller\"> </div>\n");   
  }*/
  
  for (int i = 0; i < len(groups[x]); i++) {
    fprintf(b, "\t\t<div class=\"divTable\">\n");
    fprintf(b, "\t\t\t<div class=\"divRow\">\n");
    fprintf(b, "\t\t\t\t<div class=\"divHeadCell\"> <a href=\"%s\" target=\"_blank\"> Раунд %d. Группа %d. </a> </div>\n", 
            gtables[x][i].c_str(), len(groups) - x - 1 + 1, i + 1);
    fprintf(b, "\t\t\t</div>\n");
    for (int j = 0; j < len(groups[x][i]->participants); j++) {
      fprintf(b, "\t\t\t<div class=\"divRow\">\n");
      fprintf(b, "\t\t\t\t<div class=\"%s\"> %s </div>\n", cells[j / 2], groups[x][i]->participants[j].participant->name.c_str());
      fprintf(b, "\t\t\t\t<div class=\"%s\"> %d </div>\n", cells[j / 2], groups[x][i]->participants[j].score);
      fprintf(b, "\t\t\t</div>\n");     
    }
    fprintf(b, "\t\t</div>\n");    
  }
  
  /*if (x != len(groups) - 1) {
    fprintf(b, "\t\t<div class=\"divHalfFiller\"> </div>\n");   
  }*/
  fprintf(b, "\t</div>\n");  
}

void Renderer::render(const vector<vector<Group*> >& groups)
{
  system("mkdir tables");
  vector<vector <string> > gtables(len(groups));
  
  for (int i = 0; i < len(groups); i++) {
    gtables[i].resize(len(groups[i]));    
    for (int j = 0; j < len(groups[i]); j++) {
      char s[MAXL];
      sprintf(s, "table_%02d_%02d.html", len(groups) - i - 1 + 1, j + 1);
      gtables[i][j] = string(s);
      make_group_table(s, len(groups) - i - 1 + 1, j + 1, groups[i][j]);
    }
  }
  b = fopen("./tables/results.html", "wt");
  assert(b);
  
  
  fprintf(b, "<html>\n");
  
  fprintf(b, "<head>\n");
  fprintf(b, "\t<title>Результаты</title>\n");
  fprintf(b, "\t<meta charset='utf-8'/>\n");
  fprintf(b, "\t<link rel=\"stylesheet\" type=\"text/css\" href=\"../tables.css\">\n");
  fprintf(b, "</head>\n");
  fprintf(b, "<body>\n");
  for (int i = 0; i < len(groups); i++) {
    make_round(len(groups) - i - 1, groups, gtables);
  }  
  fprintf(b, "</body>\n");
  fprintf(b, "</html>\n");
  fclose(b);
}
