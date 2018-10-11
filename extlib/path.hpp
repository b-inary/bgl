// https://github.com/dlecocq/apathy
// partially modified by b-inary (Wataru Inariba)

/******************************************************************************
 * Copyright (c) 2013 Dan Lecocq
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

#ifndef APATHY__PATH_HPP
#define APATHY__PATH_HPP

/* C++ includes */
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <istream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>

/* C includes */
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/* A class for path manipulation */
namespace apathy {
    class Path {
    public:
        /* This is the separator used on this particular system */
#ifdef __MSDOS__
        #error "Platforms using backslashes not yet supported"
#else
        static const char separator = '/';
#endif
        /* A class meant to contain path segments */
        struct Segment {
            /* The actual string segment */
            std::string segment;

            Segment(std::string s=""): segment(s) {}

            friend std::istream& operator>>(std::istream& stream, Segment& s) {
                return std::getline(stream, s.segment, separator);
            }
        };

        /**********************************************************************
         * Constructors
         *********************************************************************/

        /* Default constructor
         *
         * Points to current directory */
        Path(const std::string& path=""): path(path) {}

        /* Our generalized constructor.
         *
         * This enables all sorts of type promotion (like int -> Path) for
         * arguments into all the functions below. Anything that
         * std::stringstream can support is implicitly supported as well
         *
         * @param p - path to construct */
        template <class T>
        Path(const T& p);

        /**********************************************************************
         * Operators
         *********************************************************************/
        /* Checks if the paths are exactly the same */
        bool operator==(const Path& other) { return path == other.path; }

        /* Check if the paths are not exactly the same */
        bool operator!=(const Path& other) { return ! (*this == other); }

        /* Append the provided segment to the path as a directory. This is the
         * same as append(segment)
         *
         * @param segment - path segment to add to this path */
        Path& operator<<(const Path& segment);

        /* Append the provided segment to the path as a directory. This is the
         * same as append(segment). Returns a /new/ path object rather than a
         * reference.
         *
         * @param segment - path segment to add to this path */
        Path operator+(const Path& segment) const;

        /* Check if the two paths are equivalent
         *
         * Two paths are equivalent if they point to the same resource, even if
         * they are not exact string matches
         *
         * @param other - path to compare to */
        bool equivalent(const Path& other);

        /* Return a string version of this path */
        std::string string() const { return path; }

        /* Return the name of the file */
        std::string filename() const;

        /* Return the extension of the file */
        std::string extension() const;

        /* Return the filename without the extension */
        std::string stem() const;

        /**********************************************************************
         * Manipulations
         *********************************************************************/

        /* Append the provided segment to the path as a directory. Alias for
         * `operator<<`
         *
         * @param segment - path segment to add to this path */
        Path& append(const Path& segment);

        /* Evaluate the provided path relative to this path. If the second path
         * is absolute, then return the second path.
         *
         * @param rel - path relative to this path to evaluate */
        Path& relative(const Path& rel);

        /* Move up one level in the directory structure */
        Path& up();

        /* Turn this into an absolute path
         *
         * If the path is already absolute, it has no effect. Otherwise, it is
         * evaluated relative to the current working directory */
        Path& absolute();

        /* Sanitize this path
         *
         * This...
         *
         * 1) replaces runs of consecutive separators with a single separator
         * 2) evaluates '..' to refer to the parent directory, and
         * 3) strips out '/./' as referring to the current directory
         *
         * If the path was absolute to begin with, it will be absolute
         * afterwards. If it was a relative path to begin with, it will only be
         * converted to an absolute path if it uses enough '..'s to refer to
         * directories above the current working directory */
        Path& sanitize();

        /* Make this path a directory
         *
         * If this path does not have a trailing directory separator, add one.
         * If it already does, this does not affect the path */
        Path& directory();

        /* Trim this path of trailing separators, up to the leading separator.
         * For example, on *nix systems:
         *
         *   assert(Path("///").trim() == "/");
         *   assert(Path("/foo//").trim() == "/foo");
         */
        Path& trim();

        /**********************************************************************
         * Copiers
         *********************************************************************/

        /* Return parent path
         *
         * Returns a new Path object referring to the parent directory. To
         * move _this_ path to the parent directory, use the `up` function */
        Path parent() const { return Path(Path(*this).up()); }

        /**********************************************************************
         * Member Utility Methods
         *********************************************************************/

        /* Returns a vector of each of the path segments in this path */
        std::vector<Segment> split() const;

        /**********************************************************************
         * Type Tests
         *********************************************************************/
        /* Is the path an absolute path? */
        bool is_absolute() const;

        /* Does the path have a trailing slash? */
        bool trailing_slash() const;

        /* Does this path exist?
         *
         * Returns true if the path can be `stat`d */
        bool exists() const;

        /* Is this path an existing file?
         *
         * Only returns true if the path has stat.st_mode that is a regular
         * file */
        bool is_file() const;

        /* Is this path an existing directory?
         *
         * Only returns true if the path has a stat.st_mode that is a
         * directory */
        bool is_directory() const;

        /* How large is this file?
         *
         * Returns the file size in bytes. If the file doesn't exist, it
         * returns 0 */
        size_t size() const;

        /**********************************************************************
         * Static Utility Methods
         *********************************************************************/

        /* Return a brand new path as the concatenation of the two provided
         * paths
         *
         * @param a - first part of the path to join
         * @param b - second part of the path to join
         */
        static Path join(const Path& a, const Path& b);

        /* Return a branch new path as the concatenation of each segments
         *
         * @param segments - the path segments to concatenate
         */
        static Path join(const std::vector<Segment>& segments);

        /* Current working directory */
        static Path cwd();

        /* List all the paths in a directory
         *
         * @param p - path to list items for */
        static std::vector<Path> listdir(const Path& p);

        /* So that we can write paths out to ostreams */
        friend std::ostream& operator<<(std::ostream& stream, const Path& p) {
            return stream << p.path;
        }
    private:
        /* Our current path */
        std::string path;
    };

    /* Constructor */
    template <class T>
    inline Path::Path(const T& p): path("") {
        std::stringstream ss;
        ss << p;
        path = ss.str();
    }

    /**************************************************************************
     * Operators
     *************************************************************************/
    inline Path& Path::operator<<(const Path& segment) {
        return append(segment);
    }

    inline Path Path::operator+(const Path& segment) const {
        Path result(path);
        result.append(segment);
        return result;
    }

    inline bool Path::equivalent(const Path& other) {
        /* Make copies of both paths, sanitize, and ensure they're equal */
        return Path(path).absolute().sanitize() ==
               Path(other).absolute().sanitize();
    }

    inline std::string Path::filename() const {
        size_t pos = path.rfind(separator);
        if (pos != std::string::npos) {
            return path.substr(pos + 1);
        }
        return path;
    }

    inline std::string Path::extension() const {
        /* Make sure we only look in the filename, and not the path */
        std::string name = filename();
        if (name.empty() || name == "..") {
            return "";
        }
        size_t pos = name.substr(1).rfind('.');
        if (pos != std::string::npos) {
            return name.substr(pos + 1);
        }
        return "";
    }

    inline std::string Path::stem() const {
        std::string name = filename();
        std::string ext = extension();
        return name.substr(0, name.length() - ext.length());
    }

    /**************************************************************************
     * Manipulators
     *************************************************************************/
    inline Path& Path::append(const Path& segment) {
        /* First, check if the last character is the separator character.
         * If not, then append one and then the segment. Otherwise, just
         * the segment */
        if (!trailing_slash()) {
            path.push_back(separator);
        }
        path.append(segment.path);
        return *this;
    }

    inline Path& Path::relative(const Path& rel) {
        if (!rel.is_absolute()) {
            return append(rel).sanitize();
        } else {
            operator=(rel);
            return *this;
        }
    }

    inline Path& Path::up() {
        /* Make sure we turn this into an absolute url if it's not already
         * one */
        if (path.size() == 0) {
            path = "..";
            return directory();
        }

        append("..").sanitize();
        if (path.size() == 0) {
            return *this;
        }
        return directory();
    }

    inline Path& Path::absolute() {
        /* If the path doesn't begin with our separator, then it's not an
         * absolute path, and should be appended to the current working
         * directory */
        if (!is_absolute()) {
            /* Join our current working directory with the path */
            operator=(join(cwd(), path));
        }
        sanitize();
        return *this;
    }

    inline Path& Path::sanitize() {
        if (path.size() == 0) {
          return *this;
        }

        /* Split the path up into segments */
        std::vector<Segment> segments(split());
        /* We may have to test this repeatedly, so let's check once */
        bool relative = !is_absolute();

        /* Now, we'll create a new set of segments */
        std::vector<Segment> pruned;
        bool was_directory = false;
        for (size_t pos = 0; pos < segments.size(); ++pos) {
            /* Skip over empty segments and '.' */
            if (segments[pos].segment.size() == 0 ||
                segments[pos].segment == ".") {
                if (pos > 0) {
                    was_directory = true;
                }
                continue;
            }

            /* If there is a '..', then pop off a parent directory. However, if
             * the path was relative to begin with, if the '..'s exceed the
             * stack depth, then they should be appended to our path. If it was
             * absolute to begin with, and we reach root, then '..' has no
             * effect */
            if (segments[pos].segment == "..") {
                if (relative) {
                    if (pruned.size() && pruned.back().segment != "..") {
                        pruned.pop_back();
                    } else {
                        pruned.push_back(segments[pos]);
                    }
                } else if (pruned.size()) {
                    pruned.pop_back();
                }
                was_directory = true;
                continue;
            }

            was_directory = false;
            pruned.push_back(segments[pos]);
        }

        was_directory |= trailing_slash();
        if (!pruned.empty() && pruned.back().segment == "..") {
          was_directory = false;
        }

        if (!relative) {
            path = std::string(1, separator) + Path::join(pruned).path;
            if (was_directory) {
                return directory();
            }
            return *this;
        }

        /* It was a relative path */
        path = Path::join(pruned).path;
        if (path.length() && was_directory) {
            return directory();
        }
        if (path.empty()) {
            path = ".";
        }
        return *this;
    }

    inline Path& Path::directory() {
        trim();
        if (!trailing_slash()) {
            path.push_back(separator);
        }
        return *this;
    }

    inline Path& Path::trim() {
        if (path.length() == 0) { return *this; }

        size_t p = path.find_last_not_of(separator);
        if (p != std::string::npos) {
            path.erase(p + 1, path.size());
        } else {
            path = separator;
        }
        return *this;
    }

    /**************************************************************************
     * Member Utility Methods
     *************************************************************************/

    /* Returns a vector of each of the path segments in this path */
    inline std::vector<Path::Segment> Path::split() const {
        std::stringstream stream(path);
        std::istream_iterator<Path::Segment> start(stream);
        std::istream_iterator<Path::Segment> end;
        std::vector<Path::Segment> results(start, end);
        if (trailing_slash()) {
            results.push_back(Path::Segment(""));
        }
        return results;
    }

    /**************************************************************************
     * Tests
     *************************************************************************/
    inline bool Path::is_absolute() const {
        return path.size() && path[0] == separator;
    }

    inline bool Path::trailing_slash() const {
        return path.size() && path[path.length() - 1] == separator;
    }

    inline bool Path::exists() const {
        struct stat buf;
        if (stat(path.c_str(), &buf) != 0) {
            return false;
        }
        return true;
    }

    inline bool Path::is_file() const {
        struct stat buf;
        if (stat(path.c_str(), &buf) != 0) {
            return false;
        } else {
            return S_ISREG(buf.st_mode);
        }
    }

    inline bool Path::is_directory() const {
        struct stat buf;
        if (stat(path.c_str(), &buf) != 0) {
            return false;
        } else {
            return S_ISDIR(buf.st_mode);
        }
    }

    inline size_t Path::size() const {
        struct stat buf;
        if (stat(path.c_str(), &buf) != 0) {
            return 0;
        } else {
            return buf.st_size;
        }
    }

    /**************************************************************************
     * Static Utility Methods
     *************************************************************************/
    inline Path Path::join(const Path& a, const Path& b) {
        Path p(a);
        p.append(b);
        return p;
    }

    inline Path Path::join(const std::vector<Segment>& segments) {
        std::string path;
        /* Now, we'll go through the segments, and join them with
         * separator */
        std::vector<Segment>::const_iterator it(segments.begin());
        for(; it != segments.end(); ++it) {
            path += it->segment;
            if (it + 1 != segments.end()) {
                path += std::string(1, separator);
            }
        }
        return Path(path);
    }

    inline Path Path::cwd() {
        Path p;

        char * buf = getcwd(NULL, 0);
        if (buf != NULL) {
            p = std::string(buf);
            free(buf);
        } else {
            perror("cwd");
        }

        /* on Windows, replace backslash with slash */
        std::replace(p.path.begin(), p.path.end(), '\\', '/');

        /* Ensure this is a directory */
        p.directory();
        p.sanitize();
        return p;
    }

    /* List all the paths in a directory
     *
     * @param p - path to list items for */
    inline std::vector<Path> Path::listdir(const Path& p) {
        Path base(p);
        std::vector<Path> results;
        DIR* dir = opendir(base.string().c_str());
        if (dir == NULL) {
            /* If there was an error, return an empty vector */
            return results;
        }

        /* Otherwise, go through everything */
        for (dirent* ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
            Path cpy(base);

            /* Skip the parent directory listing */
            if (!strcmp(ent->d_name, "..")) {
                continue;
            }

            /* Skip the self directory listing */
            if (!strcmp(ent->d_name, ".")) {
                continue;
            }

            cpy.relative(ent->d_name);
            results.push_back(cpy);
        }

        errno = 0;
        closedir(dir);
        return results;
    }
}

#endif
