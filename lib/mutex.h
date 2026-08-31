//	Copyright (c) 2002 Midikyou

class CMutex{
public:

	BOOL BeginSingleBoot();
	void EndSingleBoot();
};

/*
 *	Single-instance lock (portable no-op).
 *
 *	Windows used a named mutex with timeout 0. That lock is not
 *	reproduced here; multiple instances are allowed (issue #37).
 */
inline BOOL CMutex::BeginSingleBoot(){
	return TRUE;
}

inline void CMutex::EndSingleBoot(){
}
