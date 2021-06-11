#ifndef COPY_MOVE_H
#define COPY_MOVE_H

#include "utils.h"

/**
 * @author Fabio
 * @version 1.0
 * Move or copy a file or directory from no tar to tar
 * @param source Path to file  or directory out of tar
 * @param destination Path to destination in tar
 * @param delete Boolean 1 if we delete the source, 0 don't delete
 * @return 1 Creation succes, -1 error
 **/
int moveNoTarToTar(char *source,char *destination, int delete);

/**
 * @author Fabio
 * @version 1.0
 * Move or copy a file or directory from tar to tar
 * @param source Path to file  or directory in tar
 * @param destination Path to destination in other tar
 * @param delete Boolean 1 if we delete the source, 0 don't delete
 * @return 1 Creation succes, -1 error
 **/
int moveTarToTar(char *source,char *destination, int delete);

/**
 * @author Fabio
 * @version 1.0
 * Move or copy a file or directory from tar to no tar
 * @param source Path to file or directory in  tar
 * @param destination Path to destination out of tar
 * @param delete Boolean 1 if we delete the source, 0 don't delete
 * @return 1 Creation succes, -1 error
 **/
int moveTarToNoTar(char *source,char *destination, int delete);

#endif
