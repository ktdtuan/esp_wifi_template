#include "app_config.h"
#include "sd_card.h"

#if (defined SD_CARD_ENABLE) && (SD_CARD_ENABLE == 1)

/* Includes ------------------------------------------------------------------*/
#if (defined SD_CARD_SYSTEM) && (SD_CARD_SYSTEM == 1)
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#endif

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#define SD_FS_PORT Serial
#define SD_FS_PRINTF(f_, ...) SD_FS_PORT.printf_P(PSTR(f_), ##__VA_ARGS__)
#define SD_FS_PRINTFLN(f_, ...) SD_FS_PORT.printf_P(PSTR(f_ "\r\n"), ##__VA_ARGS__)

/* Private variables ---------------------------------------------------------*/
static bool sd_card_status = false;

bool sd_card_status_isOK(void)
{
  return (true == sd_card_status);
}

void sd_card_init(void)
{
#if (defined SD_POWER_ENABLE) && (SD_POWER_ENABLE == 1)
  SD_POWER_PINMODE_INIT();
  SD_POWER_ON();
  delay(5); // add timeout to supply power to sd card
#endif
#if (defined SD_CARD_SYSTEM) && (SD_CARD_SYSTEM == 0)
  if (!SD_FS_SYSTEM.begin())
  {
    SD_FS_PRINTFLN("\r\nCard Mount Failed");
    log_report(LOG_REPORT_SD, (char *)"Card Mount Failed");
    sd_card_status = false;
    return;
  }
#else
  SD_NSS_PINMODE_INIT();
  SD_NSS_RELEASE();
  // Init SPI driver.
  // We must be init SPI PIN the first than Init SD card
  SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_NSS_PIN);
  // Init SD card with SPI clock = 80Mhz
  // https://github.com/espressif/esp-idf/issues/1008
  if (!SD_FS_SYSTEM.begin(SD_NSS_PIN, SPI, 80E6))
  {
    SD_FS_PRINTFLN("\r\nCard Mount Failed");
    log_report(LOG_REPORT_SD, (char *)"Card Mount Failed");
    sd_card_status = false;
    return;
  }
#endif
  sd_card_status = true;
  uint8_t cardType = SD_FS_SYSTEM.cardType();

  if (cardType == CARD_NONE)
  {
    SD_FS_PRINTFLN("\r\nNo SD_FS_SYSTEM card attached");
    return;
  }

  SD_FS_PRINTF("\r\nSD_FS_SYSTEM Card Type: ");
  if (cardType == CARD_MMC)
  {
    SD_FS_PRINTFLN("MMC");
  }
  else if (cardType == CARD_SD)
  {
    SD_FS_PRINTFLN("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    SD_FS_PRINTFLN("SDHC");
  }
  else
  {
    SD_FS_PRINTFLN("UNKNOWN");
  }

  uint64_t cardSize = SD_FS_SYSTEM.cardSize() / (1024 * 1024);
  SD_FS_PRINTF("SD_FS_SYSTEM Card Size: %lluMB\r\n", cardSize);

  log_report(LOG_REPORT_SD, (char *)"SD Init OK");

#if (0)
  createDir(SD_FS_SYSTEM, "/mydir");
  listDir(SD_FS_SYSTEM, "/", 0);
  removeDir(SD_FS_SYSTEM, "/mydir");
  listDir(SD_FS_SYSTEM, "/", 2);
  writeFile(SD_FS_SYSTEM, "/hello.txt", "Hello ");
  appendFile(SD_FS_SYSTEM, "/hello.txt", "World!\n");
  readFile(SD_FS_SYSTEM, "/hello.txt");
  deleteFile(SD_FS_SYSTEM, "/foo.txt");
  renameFile(SD_FS_SYSTEM, "/hello.txt", "/foo.txt");
  readFile(SD_FS_SYSTEM, "/foo.txt");
  testFileIO(SD_FS_SYSTEM, "/test.txt");
  SD_FS_PRINTF("Total space: %lluMB\n", SD_FS_SYSTEM.totalBytes() / (1024 * 1024));
  SD_FS_PRINTF("Used space: %lluMB\n", SD_FS_SYSTEM.usedBytes() / (1024 * 1024));
#endif
}

/* sd_format(SD_FS_SYSTEM, "/"); */
void sd_format(fs::FS &fs, const char *dirname)
{
  SD_FS_PRINTF("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    SD_FS_PRINTFLN("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    SD_FS_PRINTFLN("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      SD_FS_PRINTF("  DIR : %s\r\n", file.name());
      removeDir(fs, file.name());
    }
    else
    {
      SD_FS_PRINTF("  FILE: %s\r\n", file.name());
      SD_FS_PRINTF("  SIZE: %u\r\n", file.size());
      deleteFile(fs, file.name());
    }
    file = root.openNextFile();
  }
}

#if (0)
void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  SD_FS_PRINTF("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    SD_FS_PRINTFLN("Failed to open directory");
    return;
  }
  if (!root.isDirectory())
  {
    SD_FS_PRINTFLN("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      SD_FS_PRINTF("  DIR : %s\r\n", file.name());
      if (levels)
      {
        listDir(fs, file.name(), levels - 1);
      }
    }
    else
    {
      SD_FS_PRINTF("  FILE: %s\r\n", file.name());
      SD_FS_PRINTF("  SIZE: %u\r\n", file.size());
    }
    file = root.openNextFile();
  }
}
#endif

void createDir(fs::FS &fs, const char *path)
{
  SD_FS_PRINTF("Creating Dir: %s\n", path);
  if (fs.mkdir(path))
  {
    SD_FS_PRINTFLN("Dir created");
  }
  else
  {
    SD_FS_PRINTFLN("mkdir failed");
  }
}

void removeDir(fs::FS &fs, const char *path)
{
  SD_FS_PRINTF("Removing Dir: %s\n", path);
  if (fs.rmdir(path))
  {
    SD_FS_PRINTFLN("Dir removed");
  }
  else
  {
    SD_FS_PRINTFLN("rmdir failed");
  }
}

void readFile(fs::FS &fs, const char *path)
{
  SD_FS_PRINTF("Reading file: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    SD_FS_PRINTFLN("Failed to open file for reading");
    return;
  }

  SD_FS_PRINTF("Read from file: ");
  while (file.available())
  {
    SD_FS_PRINTF("%c", file.read());
  }
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  SD_FS_PRINTF("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    SD_FS_PRINTFLN("Failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    SD_FS_PRINTFLN("File written");
  }
  else
  {
    SD_FS_PRINTFLN("Write failed");
  }
}

void appendFile(fs::FS &fs, const char *path, const char *message)
{
  SD_FS_PRINTF("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    SD_FS_PRINTFLN("Failed to open file for appending");
    return;
  }
  if (file.print(message))
  {
    SD_FS_PRINTFLN("Message appended");
  }
  else
  {
    SD_FS_PRINTFLN("Append failed");
  }
}

void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  SD_FS_PRINTF("Renaming file %s to %s\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    SD_FS_PRINTFLN("File renamed");
  }
  else
  {
    SD_FS_PRINTFLN("Rename failed");
  }
}

void deleteFile(fs::FS &fs, const char *path)
{
  SD_FS_PRINTF("Deleting file: %s\n", path);
  if (fs.remove(path))
  {
    SD_FS_PRINTFLN("File deleted");
  }
  else
  {
    SD_FS_PRINTFLN("Delete failed");
  }
}

void testFileIO(fs::FS &fs, const char *path)
{
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file)
  {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len)
    {
      size_t toRead = len;
      if (toRead > 512)
      {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    SD_FS_PRINTF("%u bytes read for %u ms\n", flen, end);
    file.close();
  }
  else
  {
    SD_FS_PRINTFLN("Failed to open file for reading");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    SD_FS_PRINTFLN("Failed to open file for writing");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++)
  {
    file.write(buf, 512);
  }
  end = millis() - start;
  SD_FS_PRINTF("%u bytes written for %u ms\n", 2048 * 512, end);
  file.close();
}

#endif
