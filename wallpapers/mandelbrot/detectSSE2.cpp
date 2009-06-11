// Copyright 2007 Christopher Blauvelt <cblauvelt@gmail.com>
// Copyright 2009 by Benoît Jacob <jacob.benoit.1@gmail.com>
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License or (at your option) version 3 or any later version
// accepted by the membership of KDE e.V. (or its successor approved
// by the membership of KDE e.V.), which shall act as a proxy 
// defined in Section 14 of version 3 of the license.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//QT specific includes
#include <QList>
#include <QString>

//solid specific includes
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/processor.h>

bool system_has_SSE2()
{
  QList<Solid::Device> list = Solid::Device::listFromType(Solid::DeviceInterface::Processor, QString());

  if (list.isEmpty()) {
      // because in the magical world of Solid, we can have no CPUs in our computer
      return false;
  }

  Solid::Processor::InstructionSets extensions = list[0].as<Solid::Processor>()->instructionSets();
  return (bool)(extensions & Solid::Processor::IntelSse2);
}
