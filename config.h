/*

Session Manager.

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

// $Revision: 13859 $ $Date:: 2020-09-26 #$ $Author: serge $

#ifndef SESSION_MANAGER__CONFIG_H
#define SESSION_MANAGER__CONFIG_H

#include <cstdint>

namespace session_manager
{
struct Config
{
    uint16_t    expiration_time_min;    // in minutes
    uint16_t    max_sessions_per_user;
    bool        postpone_expiration;
};

}

#endif // SESSION_MANAGER__CONFIG_H
