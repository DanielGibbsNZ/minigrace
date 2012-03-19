package grace.lib;

import grace.lang.*;

public final class sys extends GraceObject {
  
  private static sys $module;
  public static sys $module() {
    return $module == null ? $module = new sys() : $module;
  }

  private static String[] args;
  public static void setArgs(final String[] args, final String className) {
    final int length = args.length;

    for (int i = 0; i < args.length; i++) {
      final String arg = args[i];
      if (arg.charAt(0) == '"' && arg.charAt(arg.length() - 1) == '"') {
        args[i] = arg.substring(1, arg.length() - 1);
      }
    }

    final String[] vargs = new String[length + 1];
    final String classpath = System.getProperty("java.class.path");
    vargs[0] = "java -cp " + classpath + " " + className;
    System.arraycopy(args, 0, vargs, 1, args.length);
    sys.args = vargs;
  }

  public GraceObject argv() {
    GraceList list = new GraceList();

    for (String arg : args) {
      list.push(new GraceString(arg));
    }

    return list;
  }

  public GraceObject exit(GraceObject status) {
    System.exit((int) ((GraceNumber) status).value);

    return GraceVoid.value;
  }

  public GraceObject cputime() {
    return new GraceNumber(0);
  }

  public GraceObject elapsed() {
    return new GraceNumber(0);
  }

  public GraceObject execPath() {
    return new GraceString(System.getProperty("user.dir"));
  }

}
