#include "mtk.h"

MTK_Font * MTK_Font::_self = NULL;
TTF_Font * MTK_Font::_font = NULL;
const char * MTK_Font::_fontfile = MTK_FONTFILE;

MTK_Font::MTK_Font(void)
{
  _font = TTF_OpenFont(_fontfile, 14);
}

MTK_Font::~MTK_Font(void)
{
  TTF_CloseFont(_font);
}

MTK_Font *
MTK_Font::get_instance(void)
{
  if (_self == NULL)
    _self = new MTK_Font();

  return _self;
}

TTF_Font *
MTK_Font::get_font(void)
{
  return _font;
}
