/* SPDX-License-Identifier: GPL-3.0 */

WebDispatcher.dispatcher_ready.connect(letTheShowBegin);
WebDispatcher.request_response.connect(requestResponseCallback);
WebDispatcher.dispose_replay.connect(disposeReplayableRequest);
WebDispatcher.initialize_show_request.connect(appShowRequest);
