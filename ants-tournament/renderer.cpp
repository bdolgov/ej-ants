#include "tournament.hpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cassert>

#define len(x) ((int)(x).size())
#define eprintf(...) fprintf(stderr, __VA_ARGS__)

const int MAXL = 30;

using namespace std;

const char *cells[] = {"divLoseCell", "divWinCell", "divWinCellSmooth", "divWinCellShort", "divWinCellSmoothShort"};

struct Renderer : IRenderer
{
    void render(const vector<vector<Group*> >& groups);
    void renderParticipants(const vector<Participant*>& participants);
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
    fprintf(f, "\t\t\t<div class=\"%s\">%s</div>\n", cells[group->participants[i].passed], group->participants[i].participant->name.c_str());    
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
      fprintf(f, "\t\t\t<div class=\"%s\"> %d </div>\n", cells[group->participants[j].passed], score);
    }      
    fprintf(f, "\t\t</div>\n");
  }
  
  fprintf(f, "\t\t<div class=\"divRow\">\n");
  fprintf(f, "\t\t\t<div class=\"divCell\"> Сумма </div>\n");
  for (int i = 0; i < len(group->participants); i++) {
    fprintf(f, "\t\t\t<div class=\"%s\"> %d </div>\n", cells[group->participants[i].passed], group->participants[i].score);
  }
  fprintf(f, "\t\t</div>\n");
  
  
  fprintf(f, "\t</div>\n");
  fprintf(f, "</body>\n");
  fprintf(f, "</html>\n");
  fclose(f);
}

FILE *b;

void make_round(int x, const vector<vector<Group*> >& groups, const vector<vector <string> > &gtables, const vector<vector<int> > &cords) {
  fprintf(b, "\t<div class=\"divWrap\">\n");
  /*if (x != len(groups) - 1) {
    fprintf(b, "\t\t<div class=\"divHalfFiller\"> </div>\n");   
  }*/
  
  int curx = 0;
  
  for (int i = 0; i < len(groups[x]); i++) {  
    while (curx + 2 <= cords[x][i]) {
      //fprintf(b, "\t\t<div class=\"divFiller\">  </div>\n");
      curx += 2;
    }
    while (curx + 1 <= cords[x][i]) {
      //fprintf(b, "\t\t<div class=\"divHalfFiller\"> </div>\n");     
      curx += 1;
    }
    curx += 2;
    fprintf(b, "\t\t<div class=\"divTable\">\n");
    fprintf(b, "\t\t\t<div class=\"divRow\">\n");
    fprintf(b, "\t\t\t\t<div class=\"divHeadCell\"> <a href=\"%s\" target=\"_blank\"> Раунд %d. Группа %d. </a> </div>\n", 
            gtables[x][i].c_str(), len(groups) - x - 1 + 1, i + 1);
    fprintf(b, "\t\t\t</div>\n");
    for (int j = 0; j < len(groups[x][i]->participants); j++) {
      fprintf(b, "\t\t\t<div class=\"divRow\">\n");
      fprintf(b, "\t\t\t\t<div class=\"%s\"> %s </div>\n", 
              cells[groups[x][i]->participants[j].passed], groups[x][i]->participants[j].participant->name.c_str());
      fprintf(b, "\t\t\t\t<div class=\"%s\"> %d </div>\n", 
              cells[groups[x][i]->participants[j].passed], groups[x][i]->participants[j].score);
      fprintf(b, "\t\t\t</div>\n");     
    }
    for (int j = 0; j < 4 - len(groups[x][i]->participants); j++) {
      fprintf(b, "\t\t\t<div class=\"divRow\">\n");
      fprintf(b, "\t\t\t\t<div class=\"%s\"> ---- </div>\n", cells[0]);
      fprintf(b, "\t\t\t\t<div class=\"%s\"> ---- </div>\n", cells[0]);
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
  vector<vector<int> > cords(len(groups));
  
  int k = 1;
  while (k < len(groups.back()))
    k *= 2;  
  for (int i = 0; i < k; i++) {    
    cords.back().push_back(i * 2);
  }
  for (int i = len(cords) - 2; i >= 0; i--) {
    for (int j = 0; j < len(cords[i + 1]); j += 2) {
      cords[i].push_back((cords[i + 1][j] + cords[i + 1][j + 1]) / 2);
    }
  }
  
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
  fprintf(b, "<div style=\"width:3000px\"\n>");
  
  for (int i = 0; i < len(groups); i++) {
    make_round(len(groups) - i - 1, groups, gtables, cords);
  } 
  fprintf(b, "</div>");
  fprintf(b, "</body>\n");
  fprintf(b, "</html>\n");
  fclose(b);
}

void Renderer::renderParticipants(const vector<Participant*>& participants) {
  b = fopen("./tables/participants.html", "wt");
  assert(b);
   fprintf(b, "<html>\n");
  
  fprintf(b, "<head>\n");
  fprintf(b, "\t<title>Результаты</title>\n");
  fprintf(b, "\t<meta charset='utf-8'/>\n");
  fprintf(b, "\t<link rel=\"stylesheet\" type=\"text/css\" href=\"../tables.css\">\n");
  fprintf(b, "</head>\n");
  fprintf(b, "<body link=\"#000080\" vlink=\"#000080\" alink=\"#FF0000\">\n");
  
  fprintf(b, "\t<div class=\"divTable\">\n");
  fprintf(b, "\t\t<div class=\"divRow\">\n");
  fprintf(b, "\t\t\t<div class=\"divCellShort\"> # </div>\n");
  fprintf(b, "\t\t\t<div class=\"divCell\"> Участнник </div>\n");
  fprintf(b, "\t\t\t<div class=\"divCell\"> Счет </div>\n");
  fprintf(b, "\t\t\t<div class=\"divCell\"> Предв. игра </div>\n");
  fprintf(b, "\t\t\t<div class=\"divCell\"> Предв. счет </div>\n");
  fprintf(b, "\t\t</div>\n");
  for (int i = 0; i < len(participants); i++) {
    fprintf(b, "\t\t<div class=\"divRow\">\n");
    fprintf(b, "\t\t\t<div class=\"%s\"> %d </div>\n", cells[3 + i % 2], i + 1);
    fprintf(b, "\t\t\t<div class=\"%s\"> %s </div>\n", cells[1 + i % 2], participants[i]->name.c_str());
    fprintf(b, "\t\t\t<div class=\"%s\"> %d </div>\n", cells[1 + i % 2], participants[i]->score);
    fprintf(b, "\t\t\t<div class=\"%s\"> <a href=\"http://informatics.mccme.ru/ants/visualizer.html?%d\"> Посмотреть </a></div>\n", 
            cells[1 + i % 2], participants[i]->preliminaryPlay->id);
    fprintf(b, "\t\t\t<div class=\"%s\"> %d </div>\n", cells[1 + i % 2], participants[i]->preliminaryScore);
    fprintf(b, "\t\t</div>\n");
  }
  fprintf(b, "\t</div>\n");
  fprintf(b, "</body>\n");
  fprintf(b, "</html>\n");
  
  fclose(b);
}
