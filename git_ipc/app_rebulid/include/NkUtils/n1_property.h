/**
 * N1 Э�����Զ���������ݽṹ��
 */
#include <string.h>
#include <NkUtils/types.h>
#include <NkUtils/log.h>
#include <NkUtils/macro.h>


#ifndef NK_N1_PROPERTY_H_
#define NK_N1_PROPERTY_H_
NK_CPP_EXTERN_BEGIN

/**
 * ����ѡ���ַ�����󳤶ȡ�
 */
#define NK_N1_PROP_STR_MAX_LEN (128)

/**
 * ����ѡ����������
 */
#define NK_N1_PROP_OPT_MAX_ENT (32)

/**
 * @brief �������Ͷ��塣
 */
typedef enum Nk_N1PropType
{

	NK_N1_PROP_TYPE_UNDEF = (-1),

	/**
	 * �������ԡ�
	 */
	NK_N1_PROP_TYPE_BOOL = 0,
	/**
	 * �������ԡ�
	 */
	NK_N1_PROP_TYPE_INT,
	/**
	 * ���������ԡ�
	 */
	NK_N1_PROP_TYPE_INT64,
	/**
	 * ö�������ԡ�
	 */
	NK_N1_PROP_TYPE_ENUM,
	/**
	 * ���������ԡ�
	 */
	NK_N1_PROP_TYPE_FLOAT,
	/**
	 * �ı������ԡ�
	 */
	NK_N1_PROP_TYPE_STRING,
	/**
	 * �����ַ���ԡ�
	 */
	NK_N1_PROP_TYPE_HWADDR,
	/**
	 * IPv4 ��ַ���ԡ�
	 */
	NK_N1_PROP_TYPE_IPV4,

} NK_N1PropType;


/**
 * �����������ݽṹ��
 */
typedef struct Nk_N1PropBoolean
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	NK_Boolean val;

} NK_N1PropBoolean;

/**
 * ����������ݽṹ�Ƿ�Ϸ���
 */
static inline NK_PChar
NK_N1_PROP_BOOL_CHECK(NK_N1PropBoolean *Prop)
{
	if (NK_True != Prop->val && NK_False != Prop->val) {
		return "Value Error.";
	}
	return NK_Nil;
}

/**
 * �ն˴�ӡ NK_N1PropBoolean ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline NK_Void
NK_N1_PROP_BOOL_DUMP(NK_N1PropBoolean *Prop, NK_PChar val_name)
{
	NK_TermTable Table;
	NK_PChar err = NK_Nil;

	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property Boolean" : (val_name), 32, 4);
	err = NK_N1_PROP_BOOL_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%s", Prop->val ? "True" : "False");
	NK_TermTbl_EndDraw(&Table);
}

/**
 * @brief �����������ݽṹ��
 */
typedef struct Nk_N1PropInteger
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	/**
	 * ������ֵ��
	 */
	NK_Int32 val;

	/**
	 * Ĭ����ֵֵ��\n
	 * ĳЩ�����£���������ֵ��������ʱ��Ĭ�Ͽ�����Ϊ�����ο����ݡ�
	 */
	NK_Int32 def;

	/**
	 * ���Ե����ֵ��Сֵ��\n
	 * �����ֵ����Сֵ��Ϊ 0 ��ʱ���ʾ������ֵ���������СֵԼ����\n
	 * ������ֵ��Ĭ����ֵ���봦�������Сֵ�м䣬���ԲźϷ���
	 */
	NK_Int min, max;

	/**
	 * ����ѡ�����ݽṹ��\n
	 * �� Option ��Ϊ Nil ʱ��@ref min �� @ref max ��ֵ��Ч��\n
	 * ��ֵ��ȡֵ��Χ��ѡ��Ϊ�ο���
	 */
	struct {
		NK_Size entries;
		NK_Int32 opt[NK_N1_PROP_OPT_MAX_ENT];
	} _Option, *Option;

} NK_N1PropInteger;

/**
 * ����������ݽṹ�Ƿ�Ϸ���
 */
static inline NK_PChar
NK_N1_PROP_INT_CHECK(NK_N1PropInteger *Prop)
{
	NK_Int i = 0;

	if (!Prop->Option) {
		/// ��������Ĭ��ֵ�Ƿ��ڷ�Χ�ڡ�
		if (!(0 == Prop->min && 0 == Prop->max)) {
			if (Prop->min >= Prop->max) {
				return "Range Error.";
			}
			if (Prop->val > Prop->max || Prop->val < Prop->min) {
				return "Value NOT in Range.";
			}
//			if (Prop->def > Prop->max || Prop->def < Prop->min) {
//				return "Default Value NOT in Range.";
//			}
		}
	} else {
		if (Prop->Option->entries > sizeof(Prop->Option->opt) / sizeof(Prop->Option->opt[0])
				|| !Prop->Option->entries) {

			return "Options Entires Error.";
		}
		for (i = 0; i < Prop->Option->entries; ++i) {
			if (Prop->Option->opt[i] == Prop->val) {
				break;
			}
		}
		if (i == Prop->Option->entries) {
			return "Value NOT in Options.";
		}
		for (i = 0; i < Prop->Option->entries; ++i) {
			if (Prop->Option->opt[i] == Prop->def) {
				break;
			}
		}
		if (i == Prop->Option->entries) {
			return "Default Value NOT in Options.";
		}
	}
	return NK_Nil;
}

/**
 * �ն˴�ӡ NK_N1PropInteger ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline NK_Void
NK_N1_PROP_INT_DUMP(NK_N1PropInteger *Prop, NK_PChar val_name)
{
	NK_Int i = 0;
	NK_TermTable Table;
	NK_PChar err = NK_Nil;

	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property Integer" : (val_name), 32, 4);
	err = NK_N1_PROP_INT_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%d", Prop->val);
	if (NK_Nil != Prop->Option) {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%d", Prop->def);
		NK_TermTbl_PutText(&Table, NK_True, "%-28s", "Options");
		for (i = 0; i < Prop->Option->entries; ++i) {
			NK_TermTbl_PutText(&Table, NK_False, "%28d", Prop->Option->opt[i]);
		}
	} else {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Minimum", "%d", Prop->min);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Maximum", "%d", Prop->max);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%d", Prop->def);
	}
	NK_TermTbl_EndDraw(&Table);
}

/**
 * @brief �������������ݽṹ��
 */
typedef struct Nk_N1PropInt64
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	NK_Int64 val, min, max, def;

	struct {
		NK_Size entries;
		NK_Int64 opt[NK_N1_PROP_OPT_MAX_ENT];
	} _Option, *Option;

} NK_N1PropInt64;

/**
 * ���ݽṹ��顣
 */
static inline NK_PChar
NK_N1_PROP_INT64_CHECK(NK_N1PropInt64 *Prop)
{
        NK_INT i;
	/// �ж�ȡֵ��С�Ƿ�����Ӧ��ȡֵ����
        if (Prop->Option == NK_Nil){
          if (!(0 == Prop->min && 0 == Prop->max)) {
           if (Prop->min > Prop->max){
              return "Range Error.";
           }
           if (Prop->val > Prop->max || Prop->val < Prop->min){
              return "Value Not In Range.";
           }
//           if (Prop->def > Prop->max || Prop->def < Prop->min){
//             return "Default Value Not In  Range.";
//          }
          }
        }else{
           if (Prop->Option->entries > (sizeof(Prop->Option->opt)/sizeof(Prop->Option->opt[0]))){
              return "Range error!";
           }
           for (i = 0; i < Prop->Option->entries; i++){
              if (Prop->Option->opt[i] == Prop->val){
                 break;
              }
           }
           if (i == Prop->Option->entries){
              return "Value Not In Options.";
           }

           for (i = 0; i < Prop->Option->entries; i++){
              if (Prop->Option->opt[i] == Prop->def){
                 break;
              }
           }
           if (i == Prop->Option->entries){
              return "Default Value Not In Options.";
           }
        }

	return NK_Nil;
}

/**
 * �ն˴�ӡ NK_N1PropInteger ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline NK_Void
NK_N1_PROP_INT64_DUMP(NK_N1PropInt64 *Prop, NK_PChar val_name)
{
	NK_Int i = 0;
	NK_TermTable Table;
	NK_PChar err = NK_Nil;

	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property Int64" : (val_name), 32, 4);

	err = NK_N1_PROP_INT64_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%lld", Prop->val);
	if (NK_Nil != Prop->Option) {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%lld", Prop->def);
		NK_TermTbl_PutText(&Table, NK_True, "%-28s", "Options");
		for (i = 0; i < Prop->Option->entries; ++i) {
			NK_TermTbl_PutText(&Table, NK_False, "%28lld", Prop->Option->opt[i]);
		}
	} else {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Minimum", "%lld", Prop->min);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Maximum", "%lld", Prop->max);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%lld", Prop->def);
	}
	NK_TermTbl_EndDraw(&Table);
}

/**
 * @brief ö���������ݽṹ��
 *
 * ö�������������������ԡ�\n
 * �������������ԣ�ö������û�������С�����ƣ�������߱�ѡ�\n
 * ÿ��ö��ѡ����ֵ�������һ���ı���֮��Ӧ��
 */
typedef struct Nk_N1PropEnum
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	/**
	 * ������ֵ��Ĭ����ֵ��
	 */
	NK_UInt32 val, def;

	struct {
		/**
		 * ��Чѡ��ĸ�����
		 */
		NK_Size entries;
		/**
		 * ѡ����ֵ��
		 */
		NK_UInt32 opt[NK_N1_PROP_OPT_MAX_ENT];
		/**
		 * ѡ����ֵ��Ӧ���ı���
		 */
		NK_PChar str[NK_N1_PROP_OPT_MAX_ENT];

	} _Option, *Option;

} NK_N1PropEnum;

/**
 * ���ݽṹ��顣
 */
static inline NK_PChar
NK_N1_PROP_ENUM_CHECK(NK_N1PropEnum *Prop)
{
	NK_Int i = 0;;
	/// ö�ٽṹ����ȡֵ�����ж�
	if (!Prop->Option) {
		return "Option NULL";
	}else{
		if (!(Prop->Option->entries > 0
				&& Prop->Option->entries < (sizeof(Prop->Option->opt)/sizeof(Prop->Option->opt[0])))) {
			return "Option Range Error.";
		}
		/// ��������ö��ѡ�ÿ��ö��ֵ��Ӧ�ô��ڿ��ַ�����Ӧ��
		for (i = 0; i < Prop->Option->entries; ++i) {
			if (!Prop->Option->str[i]) {
				return "Option Text Error.";
			}
		}
		/// ��������ö��ѡ�ȷ����ֵ��ö��ѡ�����ڡ�
		for (i = 0; i < Prop->Option->entries; i++) {
			if (Prop->Option->opt[i] == Prop->val) {
				break;
			}
		}
		if (i == Prop->Option->entries) {
			return "Value Not In Options.";
		}
	}
	return NK_Nil;
}
/**
 * �ն˴�ӡ NK_N1PropEnum ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline NK_Void
NK_N1_PROP_ENUM_DUMP(NK_N1PropEnum *Prop, NK_PChar val_name)
{
	NK_Int i = 0;
	NK_TermTable Table;
	NK_PChar err = NK_Nil;

	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property Enum" : (val_name), 32, 4);

	err = NK_N1_PROP_ENUM_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%u", Prop->val);
	if (NK_Nil != Prop->Option) {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%u", Prop->def);
		NK_TermTbl_PutText(&Table, NK_True, "%-28s", "Options");
		for (i = 0; i < Prop->Option->entries; ++i) {
			NK_TermTbl_PutText(&Table, NK_False, "%u: %s",
					Prop->Option->opt[i], Prop->Option->str[i]);
		}
	}
	NK_TermTbl_EndDraw(&Table);
}

/**
 * @brief �����������ݽṹ��
 */
typedef struct Nk_N1PropFloat
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	NK_DFloat val, min, max, def;

	struct {
		NK_Size entries;
		NK_DFloat opt[NK_N1_PROP_OPT_MAX_ENT];
	} _Option, *Option;

} NK_N1PropFloat;

/**
 * ���ݽṹ��顣
 */
static inline NK_PChar
NK_N1_PROP_FLOAT_CHECK(NK_N1PropFloat *Prop)
{
        NK_INT i;
	/// �������ݽṹȡֵ�ж�
        if (Prop->Option == NK_Nil) {
          if (!(0 == Prop->min && 0 == Prop->max)) {
           if (Prop->min > Prop->max) {
              return "Range error.";
           }
           if (Prop->val > Prop->max || Prop->val < Prop->min) {
              return "Value Not In Range.";
           }
//           if (Prop->def > Prop->max || Prop->def < Prop->min) {
//              return "DefValue Not In Range.";
//          }
          }
        }else{
           if (Prop->Option->entries > (sizeof(Prop->Option->opt)/sizeof(Prop->Option->opt[0]))) {
              return  "Range Error.";
           }
           for (i = 0; i < Prop->Option->entries; i++) {
              if (Prop->Option->opt[i] == Prop->val) {
                 break;
              }
           }
           if (i == Prop->Option->entries) {
              return "Value Not In Options.";
           }

           for (i = 0; i < Prop->Option->entries; i++) {
              if (Prop->Option->opt[i] == Prop->def) {
                 break;
              }
           }
           if (i == Prop->Option->entries) {
              return "Default Value Not In Options.";
           }
        }
	return NK_Nil;
}

/**
 * �ն˴�ӡ NK_N1PropEnum ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline
NK_Void NK_N1_PROP_FLOAT_DUMP(NK_N1PropFloat *Prop, NK_PChar val_name)
{
	NK_Int i = 0;
	NK_TermTable Table;
	NK_PChar err = NK_Nil;

	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property Float" : (val_name), 32, 4);

	err = NK_N1_PROP_FLOAT_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%.5f", Prop->val);
	if (NK_Nil != Prop->Option) {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%.5f", Prop->def);
		NK_TermTbl_PutText(&Table, NK_True, "%-28s", "Options");
		for (i = 0; i < Prop->Option->entries; ++i) {
			NK_TermTbl_PutText(&Table, NK_False, "%22.5f", Prop->Option->opt[i]);
		}
	} else {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Minimum", "%.5f", Prop->min);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Maximum", "%.5f", Prop->max);
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%.5f", Prop->def);
	}
	NK_TermTbl_EndDraw(&Table);
}

/**
 * �ı��������ݽṹ��
 */
typedef struct Nk_N1PropString
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	NK_Char val[NK_N1_PROP_STR_MAX_LEN + 1], def[NK_N1_PROP_STR_MAX_LEN + 1];

	/**
	 * �ı�����󳤶ȣ������� @ref NK_N1_PROP_STR_MAX_LEN��
	 * ���ı������ڴ˳������ڲźϷ���
	 */
	NK_Size max_len;

	/**
	 * �ο� NK_N1PropInteger::Option��
	 */
	struct {
		NK_Size entries;
		NK_PChar opt[NK_N1_PROP_OPT_MAX_ENT];
	} _Option, *Option;

} NK_N1PropString;

/**
 * �����ַ������ԡ�
 */
#define NK_N1_PROP_STR_SET(__Prop, __str) \
	do{\
		NK_Int i = 0;\
		if (NK_Nil != (__str)) {\
			if (NK_Nil != (__Prop)->Option) {\
				/** ��Ҫ����ѡ�������ݡ�*/\
				for (i = 0; i < (__Prop)->Option->entries; ++i) {\
					if (NK_Nil != (__Prop)->Option->opt[i] && NK_STRCMP((__Prop)->Option->opt[i], (__str))) {\
						(__Prop)->max_len = 0;\
						snprintf((__Prop)->val, sizeof((__Prop)->val), "%s", (__str));\
					}\
				}\
			} else {\
				snprintf((__Prop)->val, 0 == (__Prop)->max_len ? sizeof((__Prop)->val) : (__Prop)->max_len, "%s", (__str));\
			}\
		}\
	} while(0)

/**
 * ���ݽṹ��顣
 */
static inline NK_PChar
NK_N1_PROP_STR_CHECK(NK_N1PropString *Prop)
{
        NK_INT i;
	/// ����ַ�����ȡֵ�����ͷ�Χ
        if (Prop->max_len < 0) {
           return "String Length Value Error.";
        }
        if (Prop->max_len < strlen(Prop->val)) {
           return "Value String Length Error.";
        }
        if (Prop->max_len < strlen(Prop->def)) {
           return "Default String Length Error.";
        }

        if (Prop->Option->entries > (sizeof(Prop->Option->opt)/sizeof(Prop->Option->opt[0]))) {
           return "Range Error.";
        }

        for (i = 0; i < Prop->Option->entries; i++) {
           if (strlen(Prop->Option->opt[i]) <= Prop->max_len) {
              if (strcmp(Prop->Option->opt[i], Prop->val) == 0) {
                 break;
              }
           }
           else{
              return "Value String Length Error.";
           }
        }

        if (i == Prop->Option->entries) {
           return "Value Not In Options.";
        }

        for (i = 0;i < Prop->Option->entries; i++) {
           if (strlen(Prop->Option->opt[i]) <= Prop->max_len) {
              if (strcmp(Prop->Option->opt[i], Prop->def) == 0) {
                 break;
              }
           }
           else{
              return "Defaule Value String Length Error.";
           }
        }
        if (i == Prop->Option->entries) {
           if (i == Prop->Option->entries) {
              return "Default Value Not In Options.";
           }
        }

	return NK_Nil;
}

/**
 * �ն˴�ӡ NK_N1PropEnum ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline
NK_Void NK_N1_PROP_STRING_DUMP(NK_N1PropString *Prop, NK_PChar val_name)
{
	NK_Int i;
	NK_TermTable Table;
	NK_PChar err = NK_Nil;
	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property String" : (val_name), 32, 4);
	/// �ж��ַ����������͵�ȡֵ�ͷ�Χ
	err = NK_N1_PROP_STR_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%s", Prop->val);
	if (Prop->Option != NK_Nil) {
		NK_TermTbl_PutKeyValue(&Table, NK_True, "Default", "%s", Prop->def);
		NK_TermTbl_PutText(&Table, NK_True, "%s", "Options");
		for (i = 0; i < Prop->Option->entries; i++) {
			NK_TermTbl_PutText(&Table, NK_False, "%s", Prop->Option->opt[i]);
		}
	}
	NK_TermTbl_EndDraw(&Table);
}

/**
 * @brief �豸�����ַ�������ݽṹ��
 */
typedef struct Nk_N1PropHwAddr
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	NK_UInt8 val[6];

} NK_N1PropHwAddr;

/**
 * ���� HW ��ַ���Եĵ�ַ��
 */
#define NK_N1_PROP_HWADDR_SET(__Prop, __hw0, __hw1, __hw2, __hw3, __hw4, __hw5) \
	do{\
		(__Prop)->val[0] = (NK_UInt32)(__hw0);\
		(__Prop)->val[1] = (NK_UInt32)(__hw1);\
		(__Prop)->val[2] = (NK_UInt32)(__hw2);\
		(__Prop)->val[3] = (NK_UInt32)(__hw3);\
		(__Prop)->val[4] = (NK_UInt32)(__hw4);\
		(__Prop)->val[5] = (NK_UInt32)(__hw5);\
	} while(0)

/**
 * �ı�ת���� HW ��ַ���ԡ�
 */
#define NK_N1_PROP_HWADDR_ATON(__Prop, __hw_text) \
	do{\
		NK_Size len = strlen(__hw_text);\
		NK_Char *chr, ch;\
		NK_Int i = 0, ii = 0;\
		for (i = 0; i < 6; ++i) {\
			chr = __hw_text + i * 3;\
			if (chr < (__hw_text) + len) {\
				for (ii = 0; ii < 2; ++ii) {\
					if ((chr[ii] >= '0' && chr[ii] <= '9') \
							|| (chr[ii] >= 'a' && chr[ii] <= 'f') \
							|| (chr[ii] >= 'A' && chr[ii] <= 'F')) {\
						if (chr[ii] >= 'a' && chr[ii] <= 'f') ch = chr[ii] - 'a' + 10;\
						else if (chr[ii] >= 'A' && chr[ii] <= 'F') ch = chr[ii] - 'A' + 10;\
						else if (chr[ii] >= '0' && chr[ii] <= '9') ch = chr[ii] - '0';\
						else ch = 0;\
						\
						(__Prop)->val[i] = 0;\
						if (0 == ii) {\
							(__Prop)->val[i] |= (ch << 4);\
						} else {\
							(__Prop)->val[i] |= ch;\
						}\
					}\
				}\
			}\
		}\
	} while(0);

/**
 * HW ��ַ����ת�����ı���
 */
#define NK_N1_PROP_HWADDR_NTOA(__Prop, __text, __size) \
	snprintf(__text, (__size), "%02x:%02x:%02x:%02x:%02x:%02x",\
		(NK_UInt32)((__Prop)->val[0]),\
		(NK_UInt32)((__Prop)->val[1]),\
		(NK_UInt32)((__Prop)->val[2]),\
		(NK_UInt32)((__Prop)->val[3]),\
		(NK_UInt32)((__Prop)->val[4]),\
		(NK_UInt32)((__Prop)->val[5]))

#define NK_N1_PROP_HWADDR_STR NK_N1_PROP_HWADDR_NTOA

/**
 * ���ݽṹ��顣
 */
static inline NK_PChar
NK_N1_PROP_HWADDR_CHECK(NK_N1PropHwAddr *Prop)
{
	if ((0 == Prop->val[0] && 0 == Prop->val[1]
			&& 0 == Prop->val[2] && 0 == Prop->val[3]
			&& 0 == Prop->val[4] && 0 == Prop->val[5])
			|| (0xff == Prop->val[0] && 0xff == Prop->val[1]
					&& 0xff == Prop->val[2] && 0xff == Prop->val[3]
					&& 0xff == Prop->val[4] && 0xff == Prop->val[5])) {
		return "Ivalid Address.";
	}
	return NK_Nil;
}

/**
 * �ն˴�ӡ NK_N1PropHwAddr ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline NK_Void
NK_N1_PROP_HWADDR_DUMP(NK_N1PropHwAddr *Prop, NK_PChar val_name)
{
	NK_TermTable Table;
	NK_PChar err = NK_Nil;
	NK_Char text[32];

	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property Hardware Address" : (val_name), 64, 4);
	err = NK_N1_PROP_HWADDR_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_N1_PROP_HWADDR_NTOA(Prop, text, sizeof(text));
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%s", text);
	NK_TermTbl_EndDraw(&Table);
}


/**
 * @brief IPv4 ��ַ�������ݽṹ��
 */
typedef struct Nk_N1PropIPv4
{
	/**
	 * ����ֻ����ʶ��
	 */
	NK_Boolean read_only;

	/**
	 * �������͡�
	 */
	NK_N1PropType type;

	NK_UInt8 val[4];

} NK_N1PropIPv4;

/**
 * ���� IPv4 ���Եĵ�ַ��
 */
#define NK_N1_PROP_IPV4_SET(__Prop, __ip0, __ip1, __ip2, __ip3) \
	do{\
		(__Prop)->val[0] = (NK_UInt32)(__ip0);\
		(__Prop)->val[1] = (NK_UInt32)(__ip1);\
		(__Prop)->val[2] = (NK_UInt32)(__ip2);\
		(__Prop)->val[3] = (NK_UInt32)(__ip3);\
	} while(0)

/**
 * �ı�ת���� IPv4 �������á�
 */
#define NK_N1_PROP_IPV4_ATON(__Prop, __ipv4_text) \
	do{\
		NK_Char *ip0, *ip1, *ip2, *ip3;\
		ip0 = __ipv4_text;\
		if (NK_Nil != ip0) {\
			ip1 = strchr(ip0, '.');\
			if (NK_Nil != ip1++) {\
				ip2 = strchr(ip1, '.');\
				if (NK_Nil != ip2++) {\
					ip3 = strchr(ip2, '.');\
					if (NK_Nil != ip3++) {\
						(__Prop)->val[0] = atoi(ip0);\
						(__Prop)->val[1] = atoi(ip1);\
						(__Prop)->val[2] = atoi(ip2);\
						(__Prop)->val[3] = atoi(ip3);\
					}\
				}\
			}\
		}\
	} while(0);


/**
 * IPv4 ����ת�����ı���
 */
#define NK_N1_PROP_IPV4_NTOA(__Prop, __text, __size) \
	snprintf(__text, (__size), "%d.%d.%d.%d",\
		(NK_Int)((__Prop)->val[0]),\
		(NK_Int)((__Prop)->val[1]),\
		(NK_Int)((__Prop)->val[2]),\
		(NK_Int)((__Prop)->val[3]))

#define NK_N1_PROP_IPV4_STR NK_N1_PROP_IPV4_NTOA

/**
 * ���ݽṹ��顣
 */
static inline NK_PChar
NK_N1_PROP_IPV4_CHECK(NK_N1PropIPv4 *Prop)
{
	if (0 == Prop->val[0] && 0 == Prop->val[1]
			&& 0 == Prop->val[2] && 0 == Prop->val[3]) {
		return "Ivalid Address.";
	}
	return NK_Nil;
}

/**
 * �ն˴�ӡ NK_N1PropIPv4 ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
static inline NK_Void
NK_N1_PROP_IPV4_DUMP(NK_N1PropIPv4 *Prop, NK_PChar val_name)
{
	NK_TermTable Table;
	NK_PChar err = NK_Nil;

	NK_TermTbl_BeginDraw(&Table, !(val_name) ? "N1 Property IPv4" : (val_name), 32, 4);
	err = NK_N1_PROP_IPV4_CHECK(Prop);
	if (err) {
		NK_TermTbl_PutText(&Table, NK_True, "Error: %s", err);
	}
	NK_TermTbl_PutKeyValue(&Table, NK_True, "Value", "%d.%d.%d.%d",
			Prop->val[0], Prop->val[1], Prop->val[2], Prop->val[3]);
	NK_TermTbl_EndDraw(&Table);
}

/**
 * ���Լ��ϡ�
 */
typedef struct Nk_N1Property
{
	union {

		struct {

			/**
			 * ����ֻ����ʶ��
			 */
			NK_Boolean read_only;

			/**
			 * �������͡�
			 */
			NK_N1PropType type;
		};

		NK_N1PropBoolean Boolean;
		NK_N1PropInteger Integer;
		NK_N1PropInt64 Integer64;
		NK_N1PropEnum Enum;
		NK_N1PropFloat Float;
		NK_N1PropString String;
		NK_N1PropHwAddr HwAddr;
		NK_N1PropIPv4 IPv4;
	};

} NK_N1Property;

/**
 * ���ݽṹ��顣
 */
#define NK_N1_PROP_CHECK(__Prop) \
	((NK_N1_PROP_TYPE_BOOL == (__Prop)->type) \
	 ? NK_N1_PROP_BOOL_CHECK(&(__Prop)->Boolean) \
			:(NK_N1_PROP_TYPE_INT == (__Prop)->type) \
			 ?  NK_N1_PROP_INT_CHECK(&(__Prop)->Integer) \
					 :(NK_N1_PROP_TYPE_INT64 == (__Prop)->type) \
					  ? NK_N1_PROP_INT64_CHECK(&(__Prop)->Integer64) \
							  :(NK_N1_PROP_TYPE_ENUM == (__Prop)->type) \
							   ? NK_N1_PROP_ENUM_CHECK(&(__Prop)->Enum) \
									   :(NK_N1_PROP_TYPE_FLOAT == (__Prop)->type) \
										? NK_N1_PROP_FLOAT_CHECK(&(__Prop)->Float) \
												:(NK_N1_PROP_TYPE_STRING == (__Prop)->type) \
												 ? NK_N1_PROP_STRING_CHECK(&(__Prop)->String) \
														 :(NK_N1_PROP_TYPE_HWADDR == (__Prop)->type) \
														  ? NK_N1_PROP_HWADDR_CHECK(&(__Prop)->HwAddr) \
																  :(NK_N1_PROP_TYPE_IPV4 == (__Prop)->type) \
																   ? NK_N1_PROP_IPV4_CHECK(&(__Prop)->IPv4) \
																		   : "Invalid Property.")

/**
 * �ն˴�ӡ NK_N1PropSet ���ݽṹ��
 * ��Ҫ���ڵ��ԡ�
 */
#define NK_N1_PROP_DUMP(__Prop, __val_name) \
	do{\
		if (NK_N1_PROP_TYPE_BOOL == (__Prop)->type) {\
			NK_N1_PROP_BOOL_DUMP(&(__Prop)->Boolean, __val_name);\
		} else if (NK_N1_PROP_TYPE_INT == (__Prop)->type) {\
			NK_N1_PROP_INT_DUMP(&(__Prop)->Integer, __val_name);\
		} else if (NK_N1_PROP_TYPE_INT64 == (__Prop)->type) {\
			NK_N1_PROP_INT64_DUMP(&(__Prop)->Integer64, __val_name);\
		} else if (NK_N1_PROP_TYPE_ENUM == (__Prop)->type) {\
			NK_N1_PROP_ENUM_DUMP(&(__Prop)->Enum, __val_name);\
		} else if (NK_N1_PROP_TYPE_FLOAT == (__Prop)->type) {\
			NK_N1_PROP_FLOAT_DUMP(&(__Prop)->Float, __val_name);\
		} else if (NK_N1_PROP_TYPE_STRING == (__Prop)->type) {\
			NK_N1_PROP_STRING_DUMP(&(__Prop)->String, __val_name);\
		} else if (NK_N1_PROP_TYPE_HWADDR == (__Prop)->type) {\
			NK_N1_PROP_HWADDR_DUMP(&(__Prop)->HwAddr, __val_name);\
		} else if (NK_N1_PROP_TYPE_IPV4 == (__Prop)->type) {\
			NK_N1_PROP_IPV4_DUMP(&(__Prop)->IPv4, __val_name);\
		}\
	} while (0)

/**
 * ����׷��һ��ѡ�
 */
#define NK_N1_PROP_ADD_OPT(__Prop, __val) \
	do{\
		if (!(__Prop)->Option){\
			(__Prop)->Option = &((__Prop)->_Option);\
			(__Prop)->Option->entries = 0;\
		}\
		if ((__Prop)->Option->entries >= NK_N1_PROP_OPT_MAX_ENT){\
		/* ѡ����������� */\
			break;\
		}\
		(__Prop)->Option->opt[(__Prop)->Option->entries++] = (__val);\
	} while (0)

/**
 * ����׷��һ��ö��ѡ�
 */
#define NK_N1_PROP_ADD_ENUM(__Prop, __type, __opt) \
	do{\
		NK_Size opt_entries = NK_Nil != (__Prop)->Option ? (__Prop)->Option->entries : 0;\
		NK_N1_PROP_ADD_OPT(__Prop, __opt);\
		if (opt_entries + 1 == (__Prop)->Option->entries)\
			(__Prop)->Option->str[(__Prop)->Option->entries - 1] = NK_ENUM_MAP(__type, __opt);\
	} while(0)



NK_CPP_EXTERN_END
#endif /* NK_N1_PROPERTY_H_ */
