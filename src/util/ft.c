#include <stdio.h>
#include <math.h>

long _N;
long _L;
long _p;

int *_data;
double *_f_x;
double *_a_l;
double *_b_l;

void
ft_a(int l)
{
  int k, x_k;
  double sum;

  sum = 0.0;
  for (k = 0;k < _N;k++)
    {
      /* x_k = (k * _L / _N); */
      x_k = k;
      sum += _f_x[x_k] * cos(2.0 * M_PI * k * l / _N);
    }
  sum *= (2.0 / _N);
  _a_l[l] = sum;
}

void
ft_b(int l)
{
  int k, x_k;
  double sum, s;

  sum = 0.0;
  for (k = 0;k < _N;k++)
    {
      /* x_k = (k * _L / _N); */
      x_k = k;
      s = _f_x[x_k] * sin((2.0 * M_PI * k * l) / _N);
      sum += s;
    }
  sum *= (2.0 / _N);
  _b_l[l] = sum;
}

void
disp(long offset)
{
  int a;
  double max, min;
  int max_i, min_i;

  max = -10000000;
  min =  10000000;
  for (a = 0;a < 3;a++)
    {
      if (_a_l[a] > max)
        {
          max = _a_l[a];
          max_i = a;
        }
      if (_a_l[a] < min)
        {
          min = _a_l[a];
          min_i = a;
        }
    }
  printf("%8d | %c%13.5f%c       |  %c%13.5f%c       |  %c%13.5f%c\n",
         offset,
         (min_i == 0) ? '<' : ' ', _a_l[0], (max_i == 0) ? '*' : ' ',
         (min_i == 1) ? '<' : ' ', _a_l[1], (max_i == 1) ? '*' : ' ',
         (min_i == 2) ? '<' : ' ', _a_l[2], (max_i == 2) ? '*' : ' ');
}

void
ft(const char *filename, long offset, long len)
{
  FILE *f;
  int a, c, l;

  f = fopen(filename, "rb");
  if (!f)
    {
      printf("can't open '%s'\n", filename);
      perror("");
      exit(2);
    }

  fseek(f, offset, SEEK_SET);
  _N = len;
  _data = (int *)malloc(_N * sizeof(int));
  if (!_data)
    {
      printf("can't malloc\n");
      exit(5);
    }
  for (a = 0;a < _N;a++)
    {
      c = fgetc(f);
      if (c == EOF)
        {
          _N = a;
          break;
        }
      _data[a] = c;
    }
  fclose(f);

  _f_x = (double *)malloc(_N * sizeof(double));
  _p = (_N + 1) / 2;
  _a_l = (double *)malloc((_p + 1) * sizeof(double));
  _b_l = (double *)malloc((_p + 1) * sizeof(double));
  _L = _N;
  if ((!_f_x) || (!_a_l) || (!_b_l))
    {
      printf("can't malloc\n");
      exit(6);
    }

  // printf("N = %d, p = %d (offset = %ld, len = %ld)\n", _N, _p, offset, len);
  for (a = 0;a < _N;a++)
    _f_x[a] = (double)(_data[a] - 128);

  for (l = 0;l <= _p;l++)
    {
      _a_l[l] = 0.0;
      _b_l[l] = 0.0;
    }

  for (a = 0;a <= _p;a++)
    {
      ft_a(a);
      ft_b(a);
    }

#if 1
  for (l = _p;l >= 0;l--)
    printf("l = %3d, a = %13.5f -  b = %13.5f\n", l, _a_l[l], _b_l[l]);
#endif
#if 0
  printf("%3d: %13.5f / %13.5f / %13.5f\n", l,
         _a_l[0], _a_l[1], _a_l[2]);
#endif
#if 0
  disp(offset);
#endif

  free(_data);
  free(_f_x);
  free(_a_l);
  free(_b_l);
}

int
main(int argc, char **argv)
{
  int a;
  
  if (argc != 4)
    {
      printf("usage: %s filename offset len\n", argv[0]);
      exit(3);
    }

  for (a = 0;a < 1;a++)
    {
      ft(argv[1], atol(argv[2]) + (5 * a), atol(argv[3]));
    }

  return 0;
}
