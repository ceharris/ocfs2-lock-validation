import java.io.File;
import java.io.RandomAccessFile;
import java.nio.channels.FileChannel;
import java.nio.channels.FileLock;

class LockTest {

  private static final int DEFAULT_DELAY = 10;
  
  static class Configuration {
    public File file;
    public int delay;
  }

  public static void main(String[] args) throws Exception {
    Configuration config = configuration(args);
    if (config == null) {
      System.exit(1);
    }

    RandomAccessFile file = new RandomAccessFile(config.file, "rw");
    FileChannel channel = file.getChannel();
    FileLock lock = channel.tryLock(0L, Long.MAX_VALUE, true);
    if (lock == null) {
      System.out.println("waiting for lock");
      lock = channel.lock(0L, Long.MAX_VALUE, true);
    }

    System.out.println("lock acquired");
    System.out.format("sleeping for %d seconds\n", config.delay);
    Thread.sleep(1000 * config.delay);
    lock.release();
    System.out.println("lock released");
    file.close();
  }

  public static Configuration configuration(String[] args) {
    if (args.length < 1 || args.length > 2) {
      showUsage();
      return null;
    }

    Configuration config = new Configuration();
    config.file = new File(args[0]);
    config.delay = DEFAULT_DELAY; 

    if (args.length == 2) {
      try {
        config.delay = Integer.parseInt(args[1]);
        if (config.delay == 0) {
          showUsage();
          return null;
        }
      }    
      catch (NumberFormatException ex) {
        showUsage();
        return null;
      }
    }
   
    return config;
  }

  public static void showUsage() {
    System.err.println("usage: java LockTest filename [duration]");
  }

}
