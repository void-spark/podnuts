#ifndef SPEECH_FUNCS_H
#define SPEECH_FUNCS_H

#include <vector>

#define pod_send '\0'

class WriteUserBuff : public pod_stringbuf
{
   public:
      WriteUserBuff();
      void reset();
      void addUser(UR_OBJECT user);

   protected:
      typedef std::vector< UR_OBJECT , pod_alloc< UR_OBJECT >::Type > UsersVector;

      UsersVector users;

      virtual int        overflow(int) ;
      virtual std::streamsize xsputn(const char* s, std::streamsize n);
};

class WriteUserStream : public std::basic_ostream<char, std::char_traits<char> >
{
   private:
      static bool instanceFlag;
      static WriteUserStream *single;
      WriteUserStream();

   public:
      static WriteUserStream* getInstance();
      ~WriteUserStream();
      void addUser( UR_OBJECT user );

   protected:
      WriteUserBuff _streambuffer;
};

struct _AddUser
{
   UR_OBJECT _user;
};

inline _AddUser addUser(UR_OBJECT __adduser)
{
   _AddUser __user;
   __user._user = __adduser;
   return __user;
}

std::basic_ostream<char, std::char_traits<char> > & operator<<( std::basic_ostream<char, std::char_traits<char> > &  __os, _AddUser __user);


class WriteRoomBuff : public pod_stringbuf
{
   public:
      WriteRoomBuff();
      void reset();
      void setTargetRoom(RM_OBJECT targetRoom);
      void addUserExcept(UR_OBJECT userExcept);

   protected:
      typedef std::vector< UR_OBJECT , pod_alloc< UR_OBJECT >::Type > UsersVector;

      RM_OBJECT room;
      UsersVector exceptUsers;

      virtual int        overflow(int) ;
      virtual std::streamsize xsputn(const char* s, std::streamsize n);
};

class WriteRoomStream : public std::basic_ostream<char, std::char_traits<char> >
{
   private:
      static bool instanceFlag;
      static WriteRoomStream *single;
      WriteRoomStream();

   public:
      static WriteRoomStream* getInstance();
      ~WriteRoomStream();

      void setTargetRoom(RM_OBJECT targetRoom);
      void addUserExcept(UR_OBJECT userExcept);

   protected:
      WriteRoomBuff _streambuffer;
};

struct _SetRoom
{
   RM_OBJECT _Rm;
};

inline _SetRoom setRoom(RM_OBJECT __room)
{
   _SetRoom __r;
   __r._Rm = __room;
   return __r;
}

std::basic_ostream<char, std::char_traits<char> > & operator<<( std::basic_ostream<char, std::char_traits<char> > &  __os, _SetRoom __r);

struct _AddExcept
{
   UR_OBJECT _user;
};

inline _AddExcept addExcept(UR_OBJECT __user)
{
   _AddExcept __excpt;
   __excpt._user = __user;
   return __excpt;
}

std::basic_ostream<char, std::char_traits<char> > & operator<<( std::basic_ostream<char, std::char_traits<char> > &  __os, _AddExcept __excpt);

int  write_user      (UR_OBJECT user, const char *str);
void write_userf     (UR_OBJECT user, const char *str, ...)     __attribute__ ((format (printf,2,3)));
int  write_user_crt  (UR_OBJECT user, const char *str);
void write_user_crtf (UR_OBJECT user, const char *str, ...)     __attribute__ ((format (printf,2,3)));

int  write_room             (RM_OBJECT rm, char *str);
int  write_room_except      (RM_OBJECT rm, char *str, UR_OBJECT user);
void write_room_exceptf     (RM_OBJECT rm, UR_OBJECT user, char *str, ...)                  __attribute__ ((format (printf,3,4)));
void write_roomf            (RM_OBJECT rm, char *str, ...)                                  __attribute__ ((format (printf,2,3)));

int  write_level   (int level, int above, char *str, UR_OBJECT user);
void write_levelf  (int level, int above, UR_OBJECT user,char *str, ...) __attribute__ ((format (printf,4,5)));

void write_systemf    (char *str, ...)  __attribute__ ((format (printf,1,2)));
void write_system_admf(char *str, ...)  __attribute__ ((format (printf,1,2)));

#endif /* !SPEECH_FUNCS_H */
