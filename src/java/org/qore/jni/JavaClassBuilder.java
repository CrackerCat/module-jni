
package org.qore.jni;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.lang.reflect.Type;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

import java.nio.file.Files;
import java.nio.file.Path;

import net.bytebuddy.ByteBuddy;
import net.bytebuddy.description.modifier.Ownership;
import net.bytebuddy.description.modifier.Visibility;
import net.bytebuddy.description.type.TypeDescription;
import net.bytebuddy.dynamic.DynamicType;
import net.bytebuddy.dynamic.scaffold.subclass.ConstructorStrategy;
import net.bytebuddy.dynamic.loading.ClassLoadingStrategy;
import net.bytebuddy.implementation.bind.annotation.Argument;
import net.bytebuddy.implementation.bind.annotation.RuntimeType;
import net.bytebuddy.implementation.bytecode.assign.Assigner;
import net.bytebuddy.implementation.MethodCall;
import net.bytebuddy.NamingStrategy;

public class JavaClassBuilder {
    private static Class objArray;
    private static Method mStaticCall;
    private static Method mNormalCall;
    //private static Path tmpDir;
    private static final String CLASS_FIELD = "qore_cls_ptr";

    // copied from org.objectweb.asm.Opcodes
    public static final int ACC_PUBLIC    = (1 << 0);
    public static final int ACC_PRIVATE   = (1 << 1);
    public static final int ACC_PROTECTED = (1 << 2);
    public static final int ACC_ABSTRACT  = (1 << 10);

    // static initialization
    static {
        try {
            objArray = Class.forName("[L" + Object.class.getCanonicalName() + ";");

            Class<?>[] args = new Class<?>[3];
            args[0] = String.class;
            args[1] = long.class;
            args[2] = objArray;
            mStaticCall = JavaClassBuilder.class.getDeclaredMethod("doStaticCall", args);

            args[0] = String.class;
            args[1] = long.class;
            args[2] = objArray;
            mNormalCall = JavaClassBuilder.class.getDeclaredMethod("doNormalCall", args);
        } catch (Throwable e) {
            throw new ExceptionInInitializerError(e);
        }
    }

    static public DynamicType.Builder<?> getClassBuilder(String className, Class<?> parentClass, boolean is_abstract, long cptr)
            throws NoSuchMethodException {
        DynamicType.Builder<?> bb;
        try {
            bb = new ByteBuddy()
                .with(new NamingStrategy.AbstractBase() {
                    @Override
                    public String name(TypeDescription superClass) {
                        return className;
                    }
                })
                .subclass(parentClass, ConstructorStrategy.Default.NO_CONSTRUCTORS);
        } catch (NoClassDefFoundError e) {
            e.printStackTrace();
            throw new RuntimeException(String.format("qore-jni.jar module not in QORE_CLASSPATH; bytecode " +
                "generation unavailable; cannot perform dynamic imports in Java", e));
        }

        int modifiers = ACC_PUBLIC;
        if (is_abstract) {
            modifiers |= ACC_ABSTRACT;
        }

        bb = bb.modifiers(modifiers);

        // add a static field for storing the class ptr
        bb = (DynamicType.Builder<?>)bb.defineField(CLASS_FIELD, long.class,
            Modifier.FINAL | Modifier.PUBLIC | Modifier.STATIC)
            .value(cptr);

        return bb;
    }

    // add normal method
    static public DynamicType.Builder<?> addConstructor(DynamicType.Builder<?> bb, Class<?> parentClass, int visibility,
            List<Type> paramTypes) {
        if (paramTypes == null) {
            paramTypes = new ArrayList<Type>();
        }
        try {
            if (paramTypes.size() == 0) {
                return (DynamicType.Builder<?>)bb.defineConstructor(getVisibility(visibility))
                    .withParameters(paramTypes)
                    .throwing(Throwable.class)
                    .intercept(
                        MethodCall.invoke(parentClass.getConstructor(long.class, objArray))
                        .onSuper()
                        .withField(CLASS_FIELD)
                        .with((Object)null)
                    );
            }
            return (DynamicType.Builder<?>)bb.defineConstructor(getVisibility(visibility))
                .withParameters(paramTypes)
                .throwing(Throwable.class)
                .intercept(
                    MethodCall.invoke(parentClass.getConstructor(long.class, objArray))
                    .onSuper()
                    .withField(CLASS_FIELD)
                    .withArgumentArray()
                );
        } catch (NoSuchMethodException e) {
            throw new RuntimeException(e);
        }
    }

    // add normal method
    static public DynamicType.Builder<?> addNormalMethod(DynamicType.Builder<?> bb, String methodName, int visibility,
            Class<?> returnType, List<Type> paramTypes) {
        if (paramTypes == null) {
            paramTypes = new ArrayList<Type>();
        }

        if (paramTypes.size() == 0) {
            return (DynamicType.Builder<?>)bb.defineMethod(methodName, returnType, getVisibility(visibility),
                Ownership.MEMBER)
                .withParameters(paramTypes)
                .throwing(Throwable.class)
                .intercept(
                    MethodCall.invoke(mNormalCall)
                    .with(methodName)
                    .withField("obj")
                    .with((Object)null)
                    .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
                );
        }
        return (DynamicType.Builder<?>)bb.defineMethod(methodName, returnType, getVisibility(visibility),
            Ownership.MEMBER)
            .withParameters(paramTypes)
            .throwing(Throwable.class)
            .intercept(
                MethodCall.invoke(mNormalCall)
                .with(methodName)
                .withField("obj")
                .withAllArguments()
                .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
        );
    }

    // add static method
    static public DynamicType.Builder<?> addStaticMethod(DynamicType.Builder<?> bb, String methodName, int visibility,
            Class<?> returnType, List<Type> paramTypes) {
        return (DynamicType.Builder<?>)bb.defineMethod(methodName, returnType, getVisibility(visibility),
            Ownership.STATIC)
            .withParameters(paramTypes)
            .throwing(Throwable.class)
            .intercept(
                MethodCall.invoke(mStaticCall)
                .with(methodName)
                .withField(CLASS_FIELD)
                .withAllArguments()
                .withAssigner(Assigner.DEFAULT, Assigner.Typing.DYNAMIC)
            );
    }

    static public Class<?> getClassFromBuilder(DynamicType.Builder<?> bb, ClassLoader classLoader) {
        return bb
            .make()
            .load(classLoader, ClassLoadingStrategy.Default.WRAPPER)
            .getLoaded();
    }

    static public byte[] getByteCodeFromBuilder(DynamicType.Builder<?> bb) {
        return bb
            .make()
            .getBytes();
    }

    /*
    static public String createClassFileFromBuilder(DynamicType.Builder<?> bb) {
        checkTempDirectory();
        bb.make().saveIn(tmpDir, xxx);
    }

    static public String getTempDirectoryPath() {
        if (tmpDir == null) {
            return null;
        }
        return tmpDir.toString();
    }

    static private synchronized void checkTempDirectory() {
        if (tmpDir) {
            return;
        }

        // create the temporary directory
        tmpDir = createTempDirectory();
        // add a shutdown hook to delete all files in the temporary dir on exit
        Runtime.getRuntime().addShutdownHook(new Thread(
            new Runnable() {
                @Override
                public void run() {
                    try {
                        Files.walkFileTree(tmpDir, new SimpleFileVisitor<Path>() {
                            @Override
                            public FileVisitResult visitFile(Path file,
                                    @SuppressWarnings("unused") BasicFileAttributes attrs)
                                    throws IOException {
                                // XXX DEBUG
                                System.out.println("deleting: " + file.toString());

                                Files.delete(file);
                                return FileVisitResult.CONTINUE;
                            }
                            @Override
                            public FileVisitResult postVisitDirectory(Path dir, IOException e) throws IOException {
                                if (e == null) {
                                    Files.delete(dir);
                                    return FileVisitResult.CONTINUE;
                                }
                                // directory iteration failed
                                throw e;
                            }
                        });
                    } catch (IOException e) {
                        throw new RuntimeException("Failed to delete " + tmpDir, e);
                    }
                }
            }
        ));
    }
    */

    /** makes a static method call
     *
     * @param methodName the name of the method
     * @param args the arguments to the call, if any, can be null
     * @return the result of the call
     * @throws Throwable any exception thrown in Qore
     */
    @RuntimeType
    public static Object doStaticCall(String methodName, long qclsptr, @Argument(0) Object... args) throws Throwable {
        System.out.println(String.format("JavaClassBuilder::doStaticCall() %s() cptr: %d args: %s", methodName, qclsptr, Arrays.toString(args)));
        return doStaticCall0(methodName, qclsptr, args);
    }

    /** makes a normal method call
     *
     * @param methodName the name of the method
     * @param qobjptr the pointer to the Qore object
     * @param args the arguments to the call, if any, can be null
     * @return the result of the call
     * @throws Throwable any exception thrown in Qore
     */
    @RuntimeType
    public static Object doNormalCall(String methodName, long qobjptr, @Argument(0) Object... args) throws Throwable {
        System.out.println(String.format("JavaClassBuilder::doNormalCall() %s() ptr: %d args: %s", methodName, qobjptr, Arrays.toString(args)));
        return doNormalCall0(methodName, qobjptr, args);
    }

    static private Visibility getVisibility(int visibility) {
        switch (visibility) {
            case ACC_PUBLIC:
                return Visibility.PUBLIC;
            case ACC_PROTECTED:
                return Visibility.PROTECTED;
            default:
                break;
        }

        return Visibility.PRIVATE;
    }

    private static native Object doStaticCall0(String methodName, long qclsptr, Object... args) throws Throwable;
    private static native Object doNormalCall0(String methodName, long qobjptr, Object... args) throws Throwable;
}