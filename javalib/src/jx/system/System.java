package jx.system;
import java.util.Properties;

class System {

    public static Properties initSystemProperties(Properties properties){
        properties.put("file.encoding", "UTF-8");
        properties.put("file.separator", "/");
        properties.put("path.separator", ":");
        properties.put("line.separator", "\n");
        // properties.put("file.encoding", "US_ASCII");
        return properties;
    }
}