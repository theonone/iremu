#include "loader.hpp"

#include <cstring>
#include <iostream>

#include "../../common/errors.hpp"
#include "../../common/fileIO.hpp"
#include "../../common/tools.hpp"
#include "../cfgProv.hpp"

void ExecutableLoader::_validateELF() {
    if (_fileContents.size() < 64) {
        throw InputError("Target executable is too short");
    }
    if ((!((_fileContents[0] == 0x7F) && (_fileContents[1] == 'E') && (_fileContents[2] == 'L') &&
           (_fileContents[3] == 'F')))) {
        throw InputError("Target file is not an ELF executable");
    }
    if (!(_fileContents[4] == 0x02)) {
        throw InputError("Target ELF executable is not for 64 bit architecures");
    }
    if (!(_fileContents[5] == 0x01)) {
        throw InputError("Target ELF executable uses big endian");
    }
    if (_fileContents[0x10] != 2) {
        throw InputError("Target is ELF, but not an executable - " +
                         std::to_string(_fileContents[0x10]));
    }
    if (_fileContents[0x12] != 0x3e) {
        throw InputError("Target ELF executable is not for x86-64");
    }

    uint16_t ehsize;
    _readField(&ehsize, 0x34, 2);
    if (ehsize != 64) {
        throw InputError("Target ELF has an invalid ELF header length (not 64 bytes)");
    }
}

void ExecutableLoader::_processPhEntries() {
    uint64_t phoff;
    _readField(&phoff, 0x20, 8);
    uint16_t phentsize;
    _readField(&phentsize, 0x36, 2);
    uint16_t phnum;
    _readField(&phnum, 0x38, 2);

    if (phentsize != 56) {
        throw InputError("Wrong program header entry size. Expected 56 bytes.");
    }

    if ((phoff > _fileContents.size()) ||
        (_fileContents.size() - phoff < (uint64_t(phentsize) * uint64_t(phnum)))) {
        throw InputError("Invalid program header table");
    }

    uint64_t off = phoff;
    uint64_t firstPastSegments = 0;
    size_t loaded = 0;

    // TODO: allow page overlapping
    for (size_t i = 0; i < phnum; ++i) {
        uint32_t type;
        _readField(&type, off, 4);

        uint32_t flags;
        _readField(&flags, off + 4, 4);

        uint64_t offset;
        _readField(&offset, off + 8, 8);
        uint64_t vaddr;
        _readField(&vaddr, off + 16, 8);

        // uint64_t paddr; // irrelevant?

        uint64_t filesz;
        _readField(&filesz, off + 0x20, 8);

        uint64_t memsz;
        _readField(&memsz, off + 0x28, 8);

        uint64_t align;
        _readField(&align, off + 0x30, 8);

        if (align > 1) {
            if (((align & (align - 1)) != 0) || (vaddr % align != offset % align)) {
                throw InputError("Alignment rule not satisfied in segment at index " +
                                 std::to_string(i));
            }
        }

        if (filesz > memsz)
            throw InputError("Invalid segment size at index " + std::to_string(i));

        if (offset > _fileContents.size() || filesz > _fileContents.size() - offset)
            throw InputError("Segment exceeds file boundaries at index " + std::to_string(i));

        if (memsz > UINT64_MAX - vaddr)
            throw InputError("Segment address overflow at index " + std::to_string(i));

        // else ifs for interp and dynamic later
        if (type == 1) {
            ByteBuffer segment = _fileContents.slice(offset, offset + filesz);
            uint8_t vmemPerm = MEM_PERM_NONE;
            if (flags & 1) {
                vmemPerm |= MEM_PERM_X;
            }
            if (flags & 2) {
                vmemPerm |= MEM_PERM_W;
            }
            if (flags & 4) {
                vmemPerm |= MEM_PERM_R;
            }
            try {
                _vmem->allocate(memsz, vaddr, vmemPerm);
            } catch (const MemoryError& e) {
                throw MemoryError("Couldn't allocate " + std::to_string(memsz) +
                                  " bytes of memory for segment at index " + std::to_string(i) +
                                  ". Original message: " + e.what());
            }
            _vmem->write(vaddr, segment, true);
            if (memsz > filesz) {
                ByteBuffer zeros = {};
                zeros.resize(memsz - filesz);
                memset(zeros.data(), 0, memsz - filesz);
                _vmem->write(vaddr + filesz, zeros);
            }

            if (vaddr + memsz > firstPastSegments) {
                firstPastSegments = vaddr + memsz;
            }
            ++loaded;

        } else if (type == 2) {
            throw InputError(
                "The executable requires dynamic libraries, which are not yet supported");
        } else if (type == 3) {
            throw InputError("The executable requires an interpreter, which is not yet supported");

        } else {
            if (_verbose) {
                std::cout << "Skipping program header entry at index " << i << " - unknown type ("
                          << type << ")" << std::endl;
            }
        }

        off += phentsize;
    }

    // rounding the address up to the next page's start
    _heapBase =
        ((firstPastSegments / _pageSize) + (firstPastSegments % _pageSize != 0)) * _pageSize;

    if (loaded == 0) {
        throw InputError("The ELF contains no loadable segments");
    }
}

void ExecutableLoader::_readField(void* var, uint64_t off, uint8_t bytes) {
    if (off > _fileContents.size() || bytes > _fileContents.size() - off)
        throw InputError("Unexpected end of file");
    memcpy(var, _fileContents.cdata() + off, bytes);
}

ExecutableLoader::ExecutableLoader(const std::string& filepath, VirtualMemory* vmem,
                                   ExecutionContext ctx)
    : _ctx(ctx) {
    _fileContents = readFileAsBytes(filepath);
    _pageSize = ConfigProvider::get().page_size;
    _verbose = ConfigProvider::get().verbose;
    _vmem = vmem;
    _validateELF();
    _processPhEntries();

    uint64_t e_entry;
    _readField(&e_entry, 0x18, 8);
    ctx.regs.rip = e_entry;
    ctx.heap.setBase(_heapBase);
}
