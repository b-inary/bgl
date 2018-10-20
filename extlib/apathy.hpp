// https://github.com/dlecocq/apathy
// modified by b-inary (Wataru Inariba)

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
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cctype>

/* C includes */
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

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

            Segment(const std::string &s = "") : segment(s) {}

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
        Path(const char *path = "") : path(path) {
            sanitize();
        }

        Path(const std::string& path) : path(path) {
            sanitize();
        }

        /**********************************************************************
         * Operators
         *********************************************************************/
        /* Checks if the paths are exactly the same */
        friend bool operator==(const Path& lhs, const Path& rhs) {
            return lhs.path == rhs.path;
        }

        /* Check if the paths are not exactly the same */
        friend bool operator!=(const Path& lhs, const Path& rhs) {
            return !(lhs == rhs);
        }

        /* Append the provided segment to the path as a directory. This is the
         * same as append(segment)
         *
         * @param segment - path segment to add to this path */
        Path& operator/=(const Path& segment);

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
         * `operator/=`
         *
         * @param segment - path segment to add to this path */
        Path& append(const Path& segment);

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

        /* Return a branch new path as the concatenation of each segments
         *
         * @param segments - the path segments to concatenate
         */
        static std::string join(const std::vector<Segment>& segments);

        /* Current working directory */
        static Path cwd();

        /* Remove a file
         *
         * @param path - path to remove */
        static bool rm(const Path& p);

        static Path relative(const Path& p, const Path& base);

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

    /**************************************************************************
     * Operators
     *************************************************************************/
    inline Path& Path::operator/=(const Path& segment) {
        return append(segment);
    }

    inline Path operator/(const Path& lhs, const Path& rhs) {
        Path result(lhs);
        return result /= rhs;
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
        if (segment.is_absolute()) {
            return *this = segment;
        }
        directory();
        path.append(segment.path);
        return sanitize();
    }

    inline Path& Path::up() {
        return append("..");
    }

    inline Path& Path::absolute() {
        /* If the path doesn't begin with our separator, then it's not an
         * absolute path, and should be appended to the current working
         * directory */
        if (!is_absolute()) {
            /* Join our current working directory with the path */
            *this = cwd() / path;
        }
        return *this;
    }

    inline Path& Path::sanitize() {
        if (path.size() == 0) {
          return *this;
        }

        /* on Windows, replace backslash with slash */
        std::replace(path.begin(), path.end(), '\\', '/');

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
            path = std::string(1, separator) + Path::join(pruned);
            if (pruned.empty()) {
                path = segments[0].segment;
            }
            if (was_directory) {
                return directory();
            }
            return *this;
        }

        /* It was a relative path */
        path = Path::join(pruned);
        if (path.length() && was_directory) {
            return directory();
        }
        if (path.empty()) {
            path = ".";
        }
        return *this;
    }

    inline Path& Path::directory() {
        if (!trailing_slash()) {
            path.push_back(separator);
        }
        return *this;
    }

    inline Path& Path::trim() {
        if (path.length() != 1 && trailing_slash()) {
            path.erase(path.length() - 1);
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
        return (path.size() && path[0] == separator) ||
               (path.length() >= 3 &&
                std::isalpha(path[0]) &&path.substr(1, 2) == ":/");
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
    inline std::string Path::join(const std::vector<Segment>& segments) {
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
        return path;
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

        /* Ensure this is a directory */
        p.directory();
        p.sanitize();
        return p;
    }

    inline bool Path::rm(const Path& p) {
        if (remove(p.path.c_str()) != 0) {
            perror("Remove");
            return false;
        }
        return true;
    }

    inline Path Path::relative(const Path& p, const Path& base) {
        Path p_abs(p);
        Path base_abs(base);
        p_abs.absolute();
        base_abs.absolute().directory();

        std::vector<Segment> p_segments(p_abs.split());
        std::vector<Segment> base_segments(base_abs.split());
        base_segments.pop_back();

        while (p_segments.size() && base_segments.size() &&
               p_segments[0].segment == base_segments[0].segment) {
            p_segments.erase(p_segments.begin());
            base_segments.erase(base_segments.begin());
        }

        for (size_t pos = 0; pos < base_segments.size(); ++pos) {
            base_segments[pos].segment = "..";
        }

        base_segments.insert(base_segments.end(),
                             p_segments.begin(), p_segments.end());

        return join(base_segments);
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
            /* Skip the parent directory listing */
            if (!strcmp(ent->d_name, "..")) {
                continue;
            }

            /* Skip the self directory listing */
            if (!strcmp(ent->d_name, ".")) {
                continue;
            }

            results.push_back(base / ent->d_name);
        }

        errno = 0;
        closedir(dir);
        return results;
    }
}

#endif
