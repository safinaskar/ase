// переписать без readline? например, потому что если открыть большой файл, потом встать вверх и нажать много раз enter, то начнутся глюки
// желательно юзать пропатченый readline без SIGWINCH-ошибки

// Dolzhen byt' vyhod dazhe pri plohom getty (ctrl-c ne trebuyetsya)

// If you cannot save file, you cannot restore it (but Ctrl-L)
// Don't work with Unicode charsets
// Возможность удаления строки!!!!!!!!! Потому что это нужно мне!!!!!!!!!
// Соответствует KISS
// Не работает в странных окружениях: LFS /tools, install-debian critical (например, squeeze amd64)
// Пусть уважает input.rc!! !!
// Картинка действительно может испортиться (например, если изменить размеры терминала, то она точно портится), надо жать Ctrl-L
// убрать все лишние key bindings (которые по умолчанию в readline)?
// неправильно делает вверх, вниз при наличии табов и широких символов
// автоопределение больших файлов
// TODO: доки: единственный ;) Ctrl-S редактор (если это так)
// вы можете спросить: "почему нельзя сохранить, не выходя? почему при выходе не спрашивает о сохраниении" ответ таков: "..." (может, всё-таки исправить положение?)
// Docs: все юзают curses, оттого все редакторы полноэкранные
// mmap?
// всё таки печатать первую строчку подкрашенной, чтоб хоть как-то было понятно/удобно?
  // наверное делать весь служебный вывод подкрашенным
  // если стереть всё и нажать ctrl-l, то непонятно, что происходит
  // если набрать "e большой-файл.txt" и опечататься, то запускается ase и трудно понять, что происходит (то же самое, если маленький xD)
    // особенно если файл очень большой, и кажется что kate долго запускается, а на самом деле запустился ase
// Сделать редактор полностью нормальным, например, проверять файл на writable при открытии, а также на writable fs
// кнопки вверх и вниз должны перемещаться не между абзацами, а между строками?
// "e несуществующий-файл" создаёт этот файл! и это сбивает с толку
// просто тупо не работает даже в идеальных условиях на достаточно больших файлах
// e non-existing-dir/file.c не выдаёт предупреждения
// если "e существующий-файл" подтормаживает, то вообще невозможно понять, что происходит
// Запускаем ase и сразу же что-то набираем, потом запускается readline и текст появляется второй раз
  // Нажимаем F2, чтобы сохраниться и сразу же нажимаем что-то ещё (напр., кнопку вверх), появляются кракозябры (если комп тормозит)
// Можно сделать ase совместимым с cat, т. е. например, если stdout не tty, то просто сделать cat
// Писать "saved" или "not saved" в зависимости от способа выхода?
// Хорошо бы, чтоб весь служебный вывод был цветным и отличался от редактируемого текста. проверить, как работает ase на папках

// Ase особенно хорош на dumb terminals, например на CONFIG_VT в linux 4.0, т. к. обычные редакторы там не справляются. А вообще это повод протолкнуть в ядро какую-нибудь современную альтернативу текущему коду CONFIG_VT

/****************************************
 * cat file  +  cat > file  =  ase file *
 ****************************************/

/*  Ase
    Copyright (C) 2012  Askar Safin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <err.h>

#include <readline/readline.h>

int save;

int accept(int, int)
{
  rl_point = rl_end;
  rl_redisplay();
  rl_done = 1;
  save = 1;
  return 0;
}

int new_line(int count, int)
{
  return rl_insert(count, '\n');
}

int home(int, int)
{
  while (rl_point != 0 && rl_line_buffer[rl_point - 1] != '\n')--rl_point;
  return 0;
}

int end(int, int)
{
  while (rl_point != rl_end && rl_line_buffer[rl_point] != '\n')++rl_point;
  return 0;
}

int up(int, int)
{
  int i = 0;
  int k = rl_point;
  for (;;){
    if (k == 0)return 0;
    if (rl_line_buffer[k - 1] == '\n')break;
    if (rl_line_buffer[k - 1] == '\t'){
      i += 8;
    }else
      {
        ++i;
      }
    --k;
  }
  rl_point = k - 1;
  home(0, 0);
  for (int j = 0; j < i; ++rl_point)
    {
      if (rl_line_buffer[rl_point] == '\n')break;
      if (rl_line_buffer[rl_point] == '\t'){
        j += 8;
      }else{
        ++j;
      }
    }
  return 0;
}

int down(int, int)
{
  int i = 0;
  int k = rl_point;
  for (;;){
    if (k == 0)break;
    if (rl_line_buffer[k - 1] == '\n')break;
    if (rl_line_buffer[k - 1] == '\t'){
      i += 8;
    }else
      {
        ++i;
      }
    --k;
  }
  end(0, 0);
  if (rl_point == rl_end)return 0;
  ++rl_point;
  for (int j = 0; j < i; ++rl_point)
    {
      if (rl_point == rl_end)break;
      if (rl_line_buffer[rl_point] == '\n')break;
      if (rl_line_buffer[rl_point] == '\t'){
        j += 8;
      }else{
        ++j;
      }
    }
  return 0;
}

int cancel(int, int)
{
  rl_point = rl_end;
  rl_redisplay();
  rl_done = 1;
  return 0;
}

#define MAX_SIZE 65536

char buf[MAX_SIZE];

int init()
{
  rl_bind_key('\r', new_line); // Enter
  rl_bind_keyseq("\033\t", rl_complete); // Meta-Tab
  rl_bind_key('\t', rl_insert); // Tab
  rl_bind_keyseq("\033[1~", home); // Home (console)
  rl_bind_keyseq("\033[H", home); // Home (x)
  rl_bind_keyseq("\033OH", home); // Home (x)
  rl_bind_keyseq("\033[4~", end); // End (console)
  rl_bind_keyseq("\033[F", end); // End (x)
  rl_bind_keyseq("\033OF", end); // End (x)
  rl_bind_keyseq("\033[A", up); // Up
  rl_bind_keyseq("\033[B", down); // Down
  rl_bind_keyseq("\033[[B", accept); // F2 (console, like mc)
  rl_bind_keyseq("\033OQ", accept); // F2 (x, like mc)
  rl_bind_keyseq("\033[21~", cancel); // F10 (like mc)
  rl_bind_keyseq("\033[5~", rl_beg_of_line); // PgUp
  rl_bind_keyseq("\033[6~", rl_end_of_line); // PgDn
  rl_bind_keyseq("\4", accept); // Ctrl-D (like cat) // TODO: почему не rl_bind_key?
  rl_replace_line(buf, 0);
  return 0;
}

int main(int argc, char *argv[])
{
  int fin, fout;
  char *result;
  if (argc != 2){ fprintf(stderr, "Usage: %s FILE\n", argv[0]); exit (EXIT_FAILURE); }
  if (strcmp(argv[1], "--help") == 0){ fputs("Ase\n", stdout); exit (EXIT_SUCCESS); }
  fin = open(argv[1], O_RDONLY);
  if (fin == -1){
    if (errno != ENOENT)err (EXIT_FAILURE, "%s", argv[1]);
    buf[0] = '\0';
  }else
    {
      ssize_t size = read(fin, buf, MAX_SIZE);
      if (size == -1)err (EXIT_FAILURE, "%s", argv[1]);
      if (size == MAX_SIZE)errx (EXIT_FAILURE, "file too large");
      buf[size] = '\0';
      close(fin);
    }
  rl_startup_hook = init;
  save = 0;
  result = readline(NULL);
  if (save || result == NULL)
    {
      fout = creat(argv[1], 0666);
      if (fout == -1)err (EXIT_FAILURE, "%s", argv[1]);
      if (write(fout, result, rl_end) != rl_end)err (EXIT_FAILURE, "%s", argv[1]);
      close(fout);
    }
  free(result);
}
