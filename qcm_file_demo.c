/*****************************************************************/ /**
* @file qcm_file_demo.c
* @brief
* @author larson.li@quectel.com
* @date 2023-06-07
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description"
* <tr><td>2023-06-07 <td>1.0 <td>Larson.Li <td> Init
* </table>
**********************************************************************/

#include "qosa_log.h"
#include "qosa_def.h"
#include "qosa_sys.h"
#include "qcm_file_api.h"
#include "unirtos_app_init_registry.h"

#ifdef UNIR_EXT_FS_SUSPOORT_TEST
#include "qcm_spi_nor.h"
#endif
#include "qosa_gpio.h"
#define QOS_LOG_TAG                    LOG_TAG

#define UNIR_FILE_DEMO_TASK_STACK_SIZE 4096
//#define UNIR_EXT_FS_SUSPOORT_TEST

/**
 * @brief File directory operation demo test function
 *
 * This function demonstrates basic file system operations, including:
 * 1. Querying file system free space
 * 2. Creating directories and subdirectories
 * 3. Creating and writing test files
 * 4. Deleting entire directory tree
 */
static void unir_file_demo_test_dir(void)
{
    // Define variables used inside the function
    int          ret = 0;   // Function return value
    Q_FILE       fd = 0;    // File descriptor 1
    Q_FILE       fd2 = 0;   // File descriptor 2
    qosa_int64_t size = 0;  // File system free space size
    qosa_int32_t tmp = 0;   // Temporary variable for storing converted space size

    // Query free space size of UFS file system
    size = qcm_file_free_size("UFS");
    tmp = (qosa_int32_t)size;
    QLOGD("tmp=%d", tmp);

    // Create root test directory testdir1
    ret = qcm_file_mkdir("testdir1", 0);
    if (ret != 0 && ret != QCM_FILE_DIR_ALREADY_EXIST)
    {
        QLOGD("ret=%x", ret);
        return;
    }

    // Create subdirectories subdir1 and subdir2
    ret = qcm_file_mkdir("testdir1/subdir1", 0);
    QLOGD("ret1=%x", ret);
    ret = qcm_file_mkdir("testdir1/subdir2", 0);
    QLOGD("ret2=%x", ret);

    // Create and write test file testfile1
    fd = qcm_file_fopen("testdir1/testfile1", "w+");
    QLOGD("fd=%d", fd);
    if (fd > 0)
    {
        qcm_file_fwrite("12345", 5, 1, fd);
    }

    // Create and write test file testfile2
    fd2 = qcm_file_fopen("testdir1/testfile2", "w+");
    QLOGD("fd2=%d", fd2);
    if (fd2 > 0)
    {
        qcm_file_fwrite("12345", 5, 1, fd2);
    }

    // Close opened files
    qcm_file_fclose(fd);
    qcm_file_fclose(fd2);

    // Delete entire test directory tree, including all subdirectories and files
    ret = qcm_file_rmdir_ex("testdir1");
    QLOGD("rmdir=%x", ret);
}

/**
 * @brief File read/write function test function
 *
 * This function is used to test basic file operations, including opening, writing, reading, closing, and deleting files.
 * The main process is as follows:
 * 1. Dynamically allocate memory as data buffer;
 * 2. Create and open a file for reading and writing;
 * 3. Fill the buffer with a fixed pattern string and write data to the file;
 * 4. Reset the file pointer to the beginning of the file and get the file size;
 * 5. Clear the buffer and read data from the file;
 * 6. Print the read data content;
 * 7. Close and delete the created file;
 * 8. Release dynamically allocated memory.
 *
 * @note This function does not accept any parameters and has no return value.
 */
static void unir_file_demo_test_file(void)
{
    Q_FILE       fd = 0;                        // File descriptor
    char        *buf = QOSA_NULL;               // Buffer pointer
    qosa_int32_t buf_size = 1024;               // Buffer size
    int          temp_size = 0;                 // Currently processed data amount
    int          ret = 0;                       // Operation return value
    qosa_int32_t i = 0;                         // Loop variable
    const char  *pattern = "1234567890abcdef";  // Fill pattern string
    qosa_int32_t pattern_len = 0;               // Pattern string length

    /* Dynamically allocate buffer memory */
    buf = qosa_malloc(buf_size + 1);
    if (buf == QOSA_NULL)
    {
        QLOGE("malloc err");
        return;
    }

    /* Initialize buffer to 0 */
    qosa_memset(buf, 0, buf_size + 1);

    /* Open file for read/write operations */
    fd = qcm_file_fopen("testfile1.txt", "w+");
    if (fd < 0)
    {
        QLOGE("open err=%x", fd);
        qosa_free(buf);
        return;
    }

    /* Use pattern to cyclically fill the first buf_size bytes of buf */
    pattern_len = qosa_strlen(pattern);
    for (i = 0; i < buf_size; i++)
    {
        buf[i] = pattern[i % pattern_len];
    }

    /* Write data to file until all is written or error occurs */
    while ((temp_size < buf_size))
    {
        ret = qcm_file_fwrite(buf + temp_size, buf_size - temp_size, 1, fd);
        if (ret > 0)
        {
            temp_size += ret;
        }
        else
        {
            QLOGE("fwrite err=%x", ret);
            break;
        }
    }

    /* Move file pointer to start position and get file size */
    qcm_file_fseek(fd, 0, QCM_FILE_SEEK_SET);
    ret = qcm_file_fsize(fd);
    QLOGD("fsize=%d", ret);

    /* Reinitialize buffer, prepare to read data from file */
    temp_size = 0;
    qosa_memset(buf, 0, buf_size + 1);

    /* Read data from file until all is read or error occurs */
    while ((temp_size < buf_size))
    {
        ret = qcm_file_fread(buf + temp_size, buf_size - temp_size, 1, fd);
        if (ret > 0)
        {
            temp_size += ret;
        }
        else
        {
            QLOGE("fread err=%x", ret);
            break;
        }
    }

    /* Print the read file content */
    QLOGD("buf=%s", buf);

    /* Close file and try to delete file */
    ret = qcm_file_fclose(fd);
    QLOGD("close=%d", ret);
    ret = qcm_file_remove("testfile1.txt");
    QLOGD("remove=%d", ret);

    /* Release dynamically allocated buffer memory */
    qosa_free(buf);
}

#ifdef UNIR_EXT_FS_SUSPOORT_TEST

static void unir_ext_file_demo_test(void)
{
    Q_FILE       fd = 0;                        // File descriptor
    char        *buf = QOSA_NULL;               // Buffer pointer
    qosa_int32_t buf_size = 1024;               // Buffer size
    int          temp_size = 0;                 // Currently processed data amount
    int          ret = 0;                       // Operation return value
    qosa_int32_t i = 0;                         // Loop variable
    const char  *pattern = "1234567890abcdef";  // Fill pattern string
    qosa_int32_t pattern_len = 0;               // Pattern string length

    qcm_ext_flash_lfs_register(1); //Format when mount the file system failed

    /* Dynamically allocate buffer memory */
    buf = qosa_malloc(buf_size + 1);
    if (buf == QOSA_NULL)
    {
        QLOGE("malloc err");
        return;
    }

    /* Initialize buffer to 0 */
    qosa_memset(buf, 0, buf_size + 1);

    /* Open file for read/write operations */
    fd = qcm_file_fopen("EXTNOR:testfile1.txt", "w+");
    if (fd < 0)
    {
        QLOGE("open err=%x", fd);
        qosa_free(buf);
        return;
    }

    /* Use pattern to cyclically fill the first buf_size bytes of buf */
    pattern_len = qosa_strlen(pattern);
    for (i = 0; i < buf_size; i++)
    {
        buf[i] = pattern[i % pattern_len];
    }

    /* Write data to file until all is written or error occurs */
    while ((temp_size < buf_size))
    {
        ret = qcm_file_fwrite(buf + temp_size, buf_size - temp_size, 1, fd);
        if (ret > 0)
        {
            temp_size += ret;
        }
        else
        {
            QLOGE("fwrite err=%x", ret);
            break;
        }
    }

    /* Move file pointer to start position and get file size */
    qcm_file_fseek(fd, 0, QCM_FILE_SEEK_SET);
    ret = qcm_file_fsize(fd);
    QLOGD("fsize=%d", ret);

    /* Reinitialize buffer, prepare to read data from file */
    temp_size = 0;
    qosa_memset(buf, 0, buf_size + 1);

    /* Read data from file until all is read or error occurs */
    while ((temp_size < buf_size))
    {
        ret = qcm_file_fread(buf + temp_size, buf_size - temp_size, 1, fd);
        if (ret > 0)
        {
            temp_size += ret;
        }
        else
        {
            QLOGE("fread err=%x", ret);
            break;
        }
    }

    /* Print the read file content */
    QLOGD("buf=%s", buf);

    /* Close file and try to delete file */
    ret = qcm_file_fclose(fd);
    QLOGD("close=%d", ret);
    ret = qcm_file_remove("EXTNOR:testfile1.txt");
    QLOGD("remove=%d", ret);

    /* Release dynamically allocated buffer memory */
    qosa_free(buf);
}
#endif

/**
 * @brief This code opens a folder and traverses the current folder files and directory information
 */
void unir_file_task_handler(void *argv)
{
    QOSA_UNUSED(argv);
    qosa_task_sleep_sec(10);
    unir_file_demo_test_file();
    unir_file_demo_test_dir();
#ifdef UNIR_EXT_FS_SUSPOORT_TEST
    unir_ext_file_demo_test();
#endif
}

void unir_file_demo_init(void)
{
    int         err = 0;
    qosa_task_t file_task = QOSA_NULL;

    err = qosa_task_create(&file_task, UNIR_FILE_DEMO_TASK_STACK_SIZE, QOSA_PRIORITY_NORMAL, "file_demo", unir_file_task_handler, QOSA_NULL);
    if (err != QOSA_OK)
    {
        QLOGE("task create error");
        return;
    }
}
UNIRTOS_APP_EXPORT(324, "qcm_file_demo", unir_file_demo_init);
