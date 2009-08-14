/***************************************************************************
 *   Copyright (C) 2008-2009 by Andrzej Rybczak                            *
 *   electricityispower@gmail.com                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include "display.h"
#include "global.h"
#include "helpers.h"
#include "playlist.h"

std::string Display::Columns()
{
	if (Config.columns.empty())
		return "";
	
	std::string result, tag;
	size_t where = 0;
	int width;
	
	std::vector<Column>::const_iterator next2last;
	bool last_fixed = Config.columns.back().fixed;
	if (Config.columns.size() > 1)
		next2last = Config.columns.end()-2;
	
	for (std::vector<Column>::const_iterator it = Config.columns.begin(); it != Config.columns.end(); ++it)
	{
		width = last_fixed && it == next2last ? COLS-where-(++next2last)->width : (it->width*(it->fixed ? 1 : COLS/100.0));
		
		switch (it->type)
		{
			case 'l':
				tag = "Time";
				break;
			case 'f':
				tag = "Filename";
				break;
			case 'D':
				tag = "Directory";
				break;
			case 'a':
				tag = "Artist";
				break;
			case 't':
				tag = "Title";
				break;
			case 'b':
				tag = "Album";
				break;
			case 'y':
				tag = "Year";
				break;
			case 'n':
				tag = "Track";
				break;
			case 'g':
				tag = "Genre";
				break;
			case 'c':
				tag = "Composer";
				break;
			case 'p':
				tag = "Performer";
				break;
			case 'd':
				tag = "Disc";
				break;
			case 'C':
				tag = "Comment";
				break;
			default:
				break;
		}
		if (it->right_alignment)
			for (long i = 0; i < long(width-tag.length()-(it != Config.columns.begin())); ++i, result += ' ') { }
		
		where += width;
		result += tag;
		
		if (result.length() > where)
			result = result.substr(0, where);
		else
			for (size_t i = result.length(); i <= where && i < size_t(COLS); ++i, result += ' ') { }
	}
	return result;
}

void Display::SongsInColumns(const MPD::Song &s, void *, Menu<MPD::Song> *menu)
{
	if (!s.Localized())
		const_cast<MPD::Song *>(&s)->Localize();
	
	if (Config.columns.empty())
		return;
	
	std::vector<Column>::const_iterator next2last, it;
	size_t where = 0;
	int width;
	
	bool last_fixed = Config.columns.back().fixed;
	if (Config.columns.size() > 1)
		next2last = Config.columns.end()-2;
	
	for (it = Config.columns.begin(); it != Config.columns.end(); ++it)
	{
		if (where)
		{
			menu->GotoXY(where, menu->Y());
			*menu << ' ';
			if ((it-1)->color != clDefault)
				*menu << clEnd;
		}
		
		width = last_fixed && it == next2last ? COLS-where-(++next2last)->width : (it->width*(it->fixed ? 1 : COLS/100.0));
		MPD::Song::GetFunction get = 0;
		
		switch (it->type)
		{
			case 'l':
				get = &MPD::Song::GetLength;
				break;
			case 'D':
				get = &MPD::Song::GetDirectory;
				break;
			case 'f':
				get = &MPD::Song::GetName;
				break;
			case 'a':
				get = &MPD::Song::GetArtist;
				break;
			case 'b':
				get = &MPD::Song::GetAlbum;
				break;
			case 'y':
				get = &MPD::Song::GetDate;
				break;
			case 'n':
				get = &MPD::Song::GetTrack;
				break;
			case 'g':
				get = &MPD::Song::GetGenre;
				break;
			case 'c':
				get = &MPD::Song::GetComposer;
				break;
			case 'p':
				get = &MPD::Song::GetPerformer;
				break;
			case 'd':
				get = &MPD::Song::GetDisc;
				break;
			case 'C':
				get = &MPD::Song::GetComment;
				break;
			case 't':
				if (!s.GetTitle().empty())
					get = &MPD::Song::GetTitle;
				else
					get = &MPD::Song::GetName;
				break;
			default:
				break;
		}
		if (it->color != clDefault)
			*menu << it->color;
		whline(menu->Raw(), 32, menu->GetWidth()-where);
		std::string tag = (s.*get)();
		if (it->right_alignment)
		{
			int x, y;
			menu->GetXY(x, y);
			std::basic_string<my_char_t> wtag = TO_WSTRING(tag.empty() ? Config.empty_tag : tag).substr(0, width-!!x);
			*menu << XY(x+width-Window::Length(wtag)-!!x, y) << wtag;
		}
		else
		{
			if (!tag.empty())
				*menu << tag;
			else
				*menu << Config.empty_tag;
		}
		where += width;
	}
	if ((--it)->color != clDefault)
		*menu << clEnd;
}

void Display::Songs(const MPD::Song &s, void *data, Menu<MPD::Song> *menu)
{
	if (!s.Localized())
		const_cast<MPD::Song *>(&s)->Localize();
	
	std::string line = s.toString(*static_cast<std::string *>(data));
	for (std::string::const_iterator it = line.begin(); it != line.end(); ++it)
	{
		if (*it == '$')
		{
			if (isdigit(*++it))
			{
				*menu << Color(*it-'0');
			}
			else if (*it == 'R') // right align
			{
				basic_buffer<my_char_t> buf;
				buf << U(" ");
				String2Buffer(TO_WSTRING(line.substr(it-line.begin()+1)), buf);
				*menu << XY(COLS-buf.Str().length(), menu->Y()) << buf;
				break;
			}
			else
				*menu << *it;
		}
		else
			*menu << *it;
	}
}

void Display::Tags(const MPD::Song &s, void *data, Menu<MPD::Song> *menu)
{
	switch (static_cast<Menu<std::string> *>(data)->Choice())
	{
		case 0:
			*menu << ShowTag(s.GetTitle());
			return;
		case 1:
			*menu << ShowTag(s.GetArtist());
			return;
		case 2:
			*menu << ShowTag(s.GetAlbum());
			return;
		case 3:
			*menu << ShowTag(s.GetDate());
			return;
		case 4:
			*menu << ShowTag(s.GetTrack());
			return;
		case 5:
			*menu << ShowTag(s.GetGenre());
			return;
		case 6:
			*menu << ShowTag(s.GetComposer());
			return;
		case 7:
			*menu << ShowTag(s.GetPerformer());
			return;
		case 8:
			*menu << ShowTag(s.GetDisc());
			return;
		case 9:
			*menu << ShowTag(s.GetComment());
			return;
		case 11:
			if (s.GetNewName().empty())
				*menu << s.GetName();
			else
				*menu << s.GetName() << Config.color2 << " -> " << clEnd << s.GetNewName();
			return;
		default:
			return;
	}
}

void Display::Items(const MPD::Item &item, void *, Menu<MPD::Item> *menu)
{
	switch (item.type)
	{
		case MPD::itDirectory:
		{
			if (item.song)
			{
				*menu << "[..]";
				return;
			}
			*menu << "[" << ExtractTopDirectory(item.name) << "]";
			return;
		}
		case MPD::itSong:
			if (!Config.columns_in_browser)
				Display::Songs(*item.song, &Config.song_list_format, reinterpret_cast<Menu<MPD::Song> *>(menu));
			else
				Display::SongsInColumns(*item.song, 0, reinterpret_cast<Menu<MPD::Song> *>(menu));
			return;
		case MPD::itPlaylist:
			*menu << Config.browser_playlist_prefix << item.name;
			return;
		default:
			return;
	}
}

void Display::SearchEngine(const std::pair<Buffer *, MPD::Song *> &pair, void *, Menu< std::pair<Buffer *, MPD::Song *> > *menu)
{
	if (pair.second)
	{
		if (!Config.columns_in_search_engine)
			Display::Songs(*pair.second, &Config.song_list_format, reinterpret_cast<Menu<MPD::Song> *>(menu));
		else
			Display::SongsInColumns(*pair.second, 0, reinterpret_cast<Menu<MPD::Song> *>(menu));
	}
	
	else
		*menu << *pair.first;
}

