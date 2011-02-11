# Distributed under the Boost Software License, Version 1.0.
# See http://www.boost.org/LICENSE_1_0.txt

boost_project("Move"
  AUTHORS
    "Ion Gaztanaga"
  DESCRIPTION
    "Move Semantics"
  )

file(GLOB_RECURSE headers boost/*.* )
boost_add_headers(${headers})

add_subdirectory(doc)
