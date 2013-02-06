CONTENTS OF THIS FILE
---------------------
  * Introduction
  * Installation
  * Design Decisions

INTRODUCTION
------------
Current Maintainer: draenen (Caleb Thorne)
Sponsored in part by:
* Monarch Digital (www.monarchdigital.com)
* The North American Rock Garden Society (www.nargs.org)

Membership Entity provides a separation between memberships and user accounts.
This provides several advantages for managing membership sites.

* Memberships are fieldable separately from users.
* Attach X number of users to a single membership. (eg. Primary and secondary
  members)
* When membership is activated all users attached to the membership are given
  the active role.
* When membership expires all users attached to the membership are given the
  expired role.

INSTALLATION
------------

1. Download and install the following dependencies.
    * Date (http://drupal.org/project/date)
    * Entity reference (http://drupal.org/project/entityreference)
      Note: Entity reference was chosen over references
      (http://drupal.org/project/references) for future core compatablity.

2. Enable the module.

3.1 Create a role that will be assinged to users with an active membership.
3.2 Create a role that will be assinged to users with an expired membership.

3. Add one or more membership types at admin/structure/membership-types

4. Start creating memberships.

DESIGN DECISIONS
----------------
This module does not create any roles upon installation. This requires site
administrators to create roles that will be used for active and expired
memberships. Active/expired roles may be set on per membership type basis.
