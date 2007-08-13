#ifdef ENCYCLOPEDIA

#include <string.h>
#include <ctype.h>
#include <libxml/xmlmemory.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <SDL_types.h>

#include "../init.h"
#include "../textures.h"
#include "../symbol_table.h"
#include "symbols.h"
#include "fontdef.h"

fd_Font ** fd_fonts = NULL;
int fd_nFonts = 0;

Uint32 fd_utf8_decode(const Uint8 * src); /*!<  */
const Uint8 * fd_utf8_next_char(const Uint8 * src); /*!<  */

void fd_load() {
	char path[256];
	xmlDocPtr doc = NULL;
	xmlNodePtr rootNode, fontNode;
	int maxFont = 0;

	snprintf(path, sizeof(path), "%s/fontdef.xml", datadir);
	doc = xmlParseFile(path);
	if (doc) {
		// find root tag
		for (rootNode = doc->children; rootNode && strcmp(rootNode->name, "fontdef"); rootNode = rootNode->next);

		if (rootNode) {
			// find highest font id
			for (fontNode = rootNode->children; fontNode; fontNode = fontNode->next) {
				if (!strcmp(fontNode->name, "font")) {
					xmlChar * strval = xmlGetProp(fontNode, "id");
					if (strval) {
						int fontID = atoi(strval);
						if (fontID > maxFont) maxFont = fontID;

						xmlFree(strval);
					}
				}
			}

			// allocate space
			fd_nFonts = maxFont + 1;
			fd_fonts = calloc(fd_nFonts, sizeof(fd_Font *)); // calloc fills memory with zeroes
			bp_fonts = st_create(fd_nFonts);

			// load the fonts
			for (fontNode = rootNode->children; fontNode; fontNode = fontNode->next) {
				if (!strcmp(fontNode->name, "font")) {
					fd_Font * font = malloc(sizeof(fd_Font));
					xmlChar *strval;
					int maxChar = 0;
					xmlNodePtr texNode, charNode;

					strval = xmlGetProp(fontNode, "linesize");
					if (strval) {
						int linesize = atoi(strval);
						if (linesize > 0) {
							font->linesize = linesize;
						} else {
							LOG_ERROR("fontdef: 'linesize' attribute needs to be positive");
							free(font);
							continue;
						}
					} else {
						LOG_ERROR("fontdef: font needs to carry 'linesize' attribute");
						free(font);
						continue;
					}

					strval = xmlGetProp(fontNode, "baseline");
					if (strval) {
						int baseline = atoi(strval);
						if (baseline >= 0) {
							font->baseline = baseline;
						} else {
							LOG_ERROR("fontdef: 'baseline' attribute needs to be non-negative");
							free(font);
							continue;
						}
					} else {
						font->baseline = 0;
					}
					
					strval = xmlGetProp(fontNode, "name");
					if (strval) {
						int size = strlen(strval) + 1;
						font->name = malloc(size);
						memcpy(font->name, strval, size);
					} else {
						LOG_ERROR("fontdef: font needs to carry 'name' attribute");
						free(font);
						continue;
					}

					strval = xmlGetProp(fontNode, "id");
					if (strval) {
						int fontID = atoi(strval);
						if (fontID >= 0) {
							// add font to font and symbol table
							fd_fonts[fontID] = font;
							st_addnum(bp_fonts, font->name, fontID);
						}

						xmlFree(strval);
					} else {
						LOG_ERROR("fontdef: font needs to carry 'id' attribute");
						free(font->name);
						free(font);
						continue;
					}
					
					
					// find highest char value
					for (texNode = fontNode->children; texNode; texNode = texNode->next) {
						if (!strcmp(texNode->name, "texture")) {
							for (charNode = texNode->children; charNode; charNode = charNode->next) {
								if (!strcmp(charNode->name, "text")) {
									Uint8 * pch = charNode->content;

									if (pch) {
										while (*pch) {
											Uint32 uch = fd_utf8_decode(pch++);
											if (uch != 0xffffffff && !isspace(uch) && uch > maxChar) maxChar = uch;

											// find next line
											while (*pch && *pch != '\n') pch++;
											if (!*pch) break;
											pch++;
										}
									}
								}
							}
						}
					}

					// allocate space
					font->nChars = maxChar + 1;
					font->chars = calloc(font->nChars, sizeof(fd_Char)); // sets the array to zero

					// load the chars
					for (texNode = fontNode->children; texNode; texNode = texNode->next) {
						if (!strcmp(texNode->name, "texture")) {
							int tex;
							float texWidth, texHeight;
							char * texPath;

							texPath = xmlGetProp(texNode, "src");
							if (texPath) {
								tex = load_texture_cache_deferred(texPath,0);
								xmlFree(texPath);
							} else {
								LOG_ERROR("fontdef: texture element needs to carry 'src' attribute");
								continue;
							}

							strval = xmlGetProp(texNode, "width");
							if (strval) {
								texWidth = atof(strval);
								xmlFree(strval);
							} else {
								LOG_ERROR("fontdef: texture element needs to carry 'width' attribute");
								continue;
							}

							strval = xmlGetProp(texNode, "height");
							if (strval) {
								texHeight = atof(strval);
								xmlFree(strval);
							} else {
								LOG_ERROR("fontdef: texture element needs to carry 'height' attribute");
								continue;
							}

							for (charNode = texNode->children; charNode; charNode = charNode->next) {
								if (!strcmp(charNode->name, "text")) {
									const Uint8 * pch = charNode->content;

									if (pch) {
										while (*pch) {
											// get the char
											Uint32 uch = fd_utf8_decode(pch++);
											
											pch = fd_utf8_next_char(pch);
											
											if (uch != 0xffffffff && !isspace(uch)) {
												fd_Char * dest = &font->chars[uch];
												int left, top, width, height, baseline;

												// get the char properties
												sscanf(pch, " %i %i %i %i %i", &left, &top, &width, &height, &baseline);

												dest->tex      = tex;
												dest->left     = ((float) left) / texWidth;
												dest->top      = ((float) top)  / texHeight;
												dest->right    = ((float) left + width) / texWidth;
												dest->bottom   = ((float) top + height) / texHeight;
												dest->width    = width;
												dest->height   = height;
												dest->baseline = baseline;
												dest->valid    = 1;
											}

											// find next line
											while (*pch && *pch != '\n') pch++;
											if (!*pch) break;
											pch++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		st_commit(bp_fonts);

		free(doc);
	}
}

void fd_free() {
	if (fd_fonts) {
		int i;

		for (i=0; i < fd_nFonts; i++) {
			fd_Font * font = fd_fonts[i];

			if (font) {
				fd_fonts[i] = NULL;
				if (font->chars) free(font->chars);
				free(font);
			}
		}

		free(fd_fonts);
		fd_fonts = NULL;
	}

	fd_nFonts = 0;

	if(bp_fonts) {
		st_destroy(bp_fonts);
		bp_fonts = NULL;
	}
}

fd_Font * fd_getFont(int id) {
	if (!fd_fonts) return NULL;
	if (id > fd_nFonts) return NULL;
	return fd_fonts[id];
}

Uint32 fd_utf8_decode(const Uint8 * src) {
	switch (*src & 0xb8) {
		// MSB not set : 7 bit
		case 0x00:
		case 0x08:
		case 0x10:
		case 0x18:
		case 0x20:
		case 0x28:
		case 0x30:
		case 0x38:
			return (Uint32) *src;
		// 3rd-MSB not set : 11 bit
		case 0x80:
		case 0x88:
		case 0x90:
		case 0x98:
			return (((Uint32) src[0] & 0x1f) << 6) | ((Uint32) src[1] & 0x3f);
		// 4th-MSB not set: 16 bit
		case 0xc0:
		case 0xc8:
			return (((Uint32) src[0] & 0x0f) << 12) | (((Uint32) src[1] & 0x3f) << 6) | ((Uint32) src[2] & 0x3f);
		// 5th-MSB not set: 21 bit
		case 0xb0:
			return (((Uint32) src[0] & 0x07) << 18) |(((Uint32) src[1] & 0x3f) << 12) | (((Uint32) src[2] & 0x3f) << 6) | ((Uint32) src[3] & 0x3f);
		// all bits set: out of range
		default:
			return 0xffffffff;
	}
}

const Uint8 * fd_utf8_next_char(const Uint8 * src) {
	// skip UTF-8 trailing bytes
	while ((*src & 0xc0) == 0x80) src++;

	return src;
}

#endif /* ENCYCLOPEDIA */
