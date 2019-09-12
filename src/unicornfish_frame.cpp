// SSPHH/Fluxions/Unicornfish/Viperfish/Hatchetfish/Sunfish/Damselfish/GLUT Extensions
// Copyright (C) 2017 Jonathan Metzgar
// All rights reserved.
//
// This program is free software : you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.If not, see <https://www.gnu.org/licenses/>.
//
// For any other type of licensing, please contact me at jmetzgar@outlook.com

#include <unicornfish_frame.hpp>

namespace Uf
{
Frame::Frame()
{
}

//const Frame & Frame::operator=(const Frame & frameToCopy)
//{
//	Delete();
//	if (frameToCopy)
//		frame = zframe_dup(frameToCopy.frame);
//	return *this;
//}

const Frame &Frame::operator=(Frame &frameToMove)
{
    Delete();
    Move(&frameToMove.frame);
    frameToMove.Delete();
    return *this;
}

void Frame::Move(zframe_t **frameToMove)
{
    if (frameToMove == nullptr)
        return;
    Delete();
    frame = *frameToMove;
    *frameToMove = nullptr;
}

void Frame::Move(Frame &frameToMove)
{
    Delete();

    frame = frameToMove.frame;
    data = frameToMove.data;

    frameToMove.frame = nullptr;
    frameToMove.data.clear();
}

void Frame::Copy(const zframe_t *frameToCopy)
{
    if (frameToCopy == nullptr)
        return;
    Delete();
    frame = zframe_dup(const_cast<zframe_t *>(frameToCopy));
}

void Frame::Copy(const Frame &frameToCopy)
{
    if (frameToCopy.frame == nullptr)
        return;
    Delete();
    frame = zframe_dup(const_cast<zframe_t *>(frameToCopy.frame));
}

zframe_t *Frame::CopyZFrame() const
{
    return zframe_dup(frame);
}

void Frame::Delete()
{
    if (frame != nullptr)
        zframe_destroy(&frame);

    frame = nullptr;
    data.clear();
    hex.clear();
}

size_t Frame::SizeInBytes() const
{
    if (frame == nullptr)
        return 0;
    return zframe_size(frame);
}

bool Frame::strncmp(const char *str, size_t count) const
{
    if (!frame || SizeInBytes() < count)
        return false;
    if (::strncmp(c_str(), str, count) == 0)
        return true;
    return false;
}

void Frame::ReplaceData(const std::string &replacement)
{
    if (!frame)
        return;
    zframe_reset(frame, replacement.c_str(), replacement.size() + 1);
    data = replacement;
}

const std::string &Frame::GetStrData() const
{
    if (!frame)
        return data;
    if (data.empty())
    {
        char *str = zframe_strdup(frame);
        data = str;
        zstr_free(&str);
    }
    return data;
}

const std::string &Frame::GetHexData() const
{
    if (!frame)
        return hex;
    if (hex.empty())
    {
        char *str = zframe_strhex(frame);
        hex = str;
        zstr_free(&str);
    }
    return hex;
}

const void *Frame::GetData() const
{
    if (!frame)
        return nullptr;
    void *pdata = zframe_data(frame);
    return pdata;
}

zframe_t *Frame::MoveZFrame()
{
    zframe_t *tmp = frame;
    frame = nullptr;
    Delete();
    return tmp;
}
} // namespace Uf