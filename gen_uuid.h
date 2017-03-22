/*

UUID generator.

Copyright (C) 2016 Sergey Kolevatov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

*/

// $Revision: 3577 $ $Date:: 2016-03-31 #$ $Author: serge $

#ifndef SESSION_MANAGER_GEN_UUID_H
#define SESSION_MANAGER_GEN_UUID_H

#include <string>       // std::string

namespace session_manager
{
std::string gen_uuid();
}

#endif // SESSION_MANAGER_GEN_UUID_H
